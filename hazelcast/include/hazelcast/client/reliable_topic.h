/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <string>
#include <memory>
#include <atomic>

#include "hazelcast/client/ringbuffer.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/concurrent/Cancellable.h"
#include "hazelcast/logger.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicMessage.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {

/**
 * Hazelcast provides distribution mechanism for publishing messages that are
 * delivered to multiple subscribers which is also known as publish/subscribe
 * (pub/sub) messaging model. Publish and subscriptions are cluster-wide. When a
 * member subscribes for a topic, it is actually registering for messages
 * published by any member in the cluster, including the new members joined
 * after you added the listener.
 *
 * Messages are ordered, meaning, listeners(subscribers)
 * will process the messages in the order they are actually published. If
 * cluster member M publishes messages m1, m2, m3...mn to a topic T, then
 * Hazelcast makes sure that all of the subscribers of topic T will receive and
 * process m1, m2, m3...mn in order.
 *
 */
class HAZELCAST_API reliable_topic
  : public proxy::ProxyImpl
  , public std::enable_shared_from_this<reliable_topic>
{
    friend class spi::ProxyManager;
    friend class hazelcast_client;

public:
    static constexpr const char* SERVICE_NAME = "hz:impl:reliableTopicService";

    /**
     * Publishes the message to all subscribers of this topic
     * Current implementation only supports DISCARD_OLDEST policy as in Java
     * client. The other policies will be available when async API is completed.
     * Using this policy the oldest item is overwritten no matter it is not old
     * enough to retire.
     *
     * @param message The message to be published
     */
    template<typename E>
    boost::future<void> publish(const E& message)
    {
        topic::impl::reliable::ReliableTopicMessage reliable_message(
          to_data(message), nullptr);
        return to_void_future(ringbuffer_.get()->add(reliable_message));
    }

    /**
     * Subscribes to this topic. When someone publishes a message on this topic.
     * onMessage() function of the given MessageListener is called. More than
     * one message listener can be added on one instance.
     *
     * Warning 1: If listener should do a time consuming operation, off-load the
     * operation to another thread. otherwise it will slow down the system.
     *
     * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
     *
     * Warning 3: Make sure that the MessageListener object is not destroyed
     * until the removeListener is called, since the library will use the
     * MessageListener reference to deliver incoming messages.
     *
     * @param listener the MessageListener to add.
     *
     * @return returns registration id.
     */
    template<typename Listener>
    std::string add_message_listener(Listener&& listener)
    {
        int id = ++runner_counter_;
        std::shared_ptr<MessageRunner<Listener>> runner(
          new MessageRunner<Listener>(id,
                                      std::forward<Listener>(listener),
                                      ringbuffer_.get(),
                                      get_name(),
                                      get_serialization_service(),
                                      batch_size_,
                                      logger_,
                                      execution_service_,
                                      executor_,
                                      runners_map_,
                                      shared_from_this()));
        runners_map_.put(id, runner);
        runner->next();
        return std::to_string(id);
    }

    /**
     * Stops receiving messages for the given message listener. If the given
     * listener already removed, this method does nothing.
     *
     * @param registrationId Id of listener registration.
     *
     * @return true if registration is removed, false otherwise
     */
    bool remove_message_listener(const std::string& registration_id);

protected:
    void on_shutdown() override;

    void on_destroy() override;

    void post_destroy() override;

private:
    static constexpr const char* TOPIC_RB_PREFIX = "_hz_rb_";

    reliable_topic(const std::string& instance_name,
                   spi::ClientContext* context);

    template<typename Listener>
    class MessageRunner
      : public std::enable_shared_from_this<MessageRunner<Listener>>
      , public util::concurrent::Cancellable
    {
    public:
        MessageRunner(int id,
                      Listener&& listener,
                      const std::shared_ptr<ringbuffer>& rb,
                      const std::string& topic_name,
                      serialization::pimpl::SerializationService& service,
                      int batch_size,
                      logger& lg,
                      std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
                        execution_service,
                      util::hz_thread_pool& executor,
                      util::SynchronizedMap<int, util::concurrent::Cancellable>&
                        runners_map,
                      std::shared_ptr<reliable_topic> topic)
          : listener_(listener)
          , id_(id)
          , ringbuffer_(rb)
          , cancelled_(false)
          , logger_(lg)
          , name_(topic_name)
          , execution_service_(std::move(execution_service))
          , executor_(executor)
          , serialization_service_(service)
          , batch_size_(batch_size)
          , runners_map_(runners_map)
          , topic_(std::move(topic))
        {
            // we are going to listen to next publication. We don't care about
            // what already has been published.
            int64_t initialSequence = listener.initial_sequence_id_;
            if (initialSequence == -1) {
                initialSequence = ringbuffer_->tail_sequence().get() + 1;
            }
            sequence_ = initialSequence;
        }

        virtual ~MessageRunner() = default;

        void next()
        {
            if (cancelled_) {
                return;
            }

            auto runner = this->shared_from_this();
            auto execution_service = this->execution_service_;            
            ringbuffer_->read_many(sequence_, 1, batch_size_)
              .then(executor_, [runner,execution_service](boost::future<rb::read_result_set> f) {
                  if (runner->cancelled_) {
                      return;
                  }

                  try {
                      auto result = f.get();

                      // we process all messages in batch. So we don't release
                      // the thread and reschedule ourselves; but we'll process
                      // whatever was received in 1 go.
                      auto lost_count =
                        result.get_next_sequence_to_read_from() -
                        result.read_count() - runner->sequence_;
                      if (lost_count != 0 &&
                          !runner->listener_.loss_tolerant_) {
                          runner->cancel();
                          return;
                      }

                      auto const& items = result.get_items();
                      for (size_t i = 0; i < items.size(); i++) {
                          auto const& message = items[i];
                          try {
                              runner->listener_.store_sequence_id_(
                                result.get_sequence(static_cast<int>(i)));
                              auto rel_msg = message.get<
                                topic::impl::reliable::ReliableTopicMessage>();
                              runner->process(*rel_msg);
                          } catch (exception::iexception& e) {
                              if (runner->terminate(e)) {
                                  runner->cancel();
                                  return;
                              }
                          }
                      }

                      runner->sequence_ =
                        result.get_next_sequence_to_read_from();
                      runner->next();
                  } catch (exception::iexception& ie) {
                      if (runner->handle_internal_exception(ie)) {
                          runner->next();
                      } else {
                          runner->cancel();
                      }
                  }
              });
        }

        bool handle_operation_timeout_exception()
        {
            HZ_LOG(logger_,
                   finest,
                   boost::str(
                     boost::format("MessageListener on topic: %1% timed out. "
                                   "Continuing from last known sequence: %2%") %
                     name_ % sequence_));

            return true;
        }

        /**
         * Handles the \illegal_argument_exception associated with requesting
         * a sequence larger than the tail_sequence() + 1.
         * This may indicate that an entire partition or an entire ringbuffer
         * was lost.
         *
         * @param e the exception
         * @return if the exception was handled and the listener may continue
         * reading
         */
        bool handle_illegal_argument_exception(exception::iexception& e)
        {
            // stale_sequence_exception.getHeadSeq() is not available on the
            // client-side, see #7317
            int64_t remoteHeadSeq = ringbuffer_->head_sequence().get();

            if (listener_.loss_tolerant_) {
                HZ_LOG(logger_,
                       finest,
                       boost::str(
                         boost::format(
                           "Terminating MessageListener: %1% on topic: %2% ."
                           "Reason: Underlying ring buffer data related to "
                           "reliable topic is lost.") %
                         id_ % name_ % e.what() % sequence_ % remoteHeadSeq));
                sequence_ = remoteHeadSeq;
                return true;
            }

            HZ_LOG(
              logger_,
              warning,
              boost::str(boost::format(
                           "Terminating MessageListener: %1% on topic: %2%"
                           "Reason: The listener was too slow or the retention "
                           "period of the message has been violated. "
                           "head: %3% sequence: %4%") %
                         id_ % name_ % remoteHeadSeq % sequence_));
            return false;
        }

        /**
         * @param ie exception to check if it is terminal or can be handled so
         * that topic can continue
         * @return true if the exception was handled and the listener may
         * continue reading
         */
        bool handle_internal_exception(exception::iexception& ie)
        {
            int32_t err = ie.get_error_code();

            switch (err) {
                case protocol::TIMEOUT:
                    return handle_operation_timeout_exception();
                case protocol::ILLEGAL_ARGUMENT:
                    return handle_illegal_argument_exception(ie);
                case protocol::HAZELCAST_INSTANCE_NOT_ACTIVE:
                    HZ_LOG(logger_,
                           finest,
                           boost::str(
                             boost::format(
                               "Terminating MessageListener %1% on topic: %2%. "
                               "Reason: HazelcastInstance is shutting down") %
                             id_ % name_));
                case protocol::DISTRIBUTED_OBJECT_DESTROYED:
                    HZ_LOG(logger_,
                           finest,
                           boost::str(
                             boost::format(
                               "Terminating MessageListener %1% on topic: %2%. "
                               "Reason: Topic is destroyed") %
                             id_ % name_));
                default:
                    HZ_LOG(logger_,
                           warning,
                           boost::str(
                             boost::format(
                               "Terminating MessageListener %1% on topic: %2%. "
                               "Reason: Unhandled exception, details: %3%") %
                             id_ % name_ % ie.what()));
            }
            return false;
        }

        bool cancel() override
        {
            cancelled_.store(true);
            runners_map_.remove(id_);
            return true;
        }

        bool is_cancelled() override { return cancelled_.load(); }

    private:
        void process(topic::impl::reliable::ReliableTopicMessage& message)
        {
            listener_.received_(to_message(message));
        }

        topic::message to_message(
          topic::impl::reliable::ReliableTopicMessage& message)
        {
            boost::optional<member> m;
            auto& addr = message.get_publisher_address();
            if (addr.has_value()) {
                m = boost::make_optional<member>(addr.value());
            }
            return topic::message(name_,
                                  typed_data(std::move(message.get_payload()),
                                             serialization_service_),
                                  message.get_publish_time(),
                                  std::move(m));
        }

        bool terminate(const exception::iexception& failure)
        {
            if (cancelled_) {
                return true;
            }

            try {
                bool terminate = listener_.terminal_(failure);
                if (terminate) {
                    HZ_LOG(logger_,
                           warning,
                           boost::str(
                             boost::format(
                               "Terminating ReliableListener %1% "
                               "on topic: %2%. "
                               "Reason: Unhandled exception, details: %3%") %
                             id_ % name_ % failure.what()));
                } else {
                    HZ_LOG(logger_,
                           finest,
                           boost::str(boost::format(
                                        "ReliableListener %1% on topic: %2% "
                                        "ran into an exception, details: %3%") %
                                      id_ % name_ % failure.what()));
                }
                return terminate;
            } catch (exception::iexception& t) {
                HZ_LOG(logger_,
                       warning,
                       boost::str(
                         boost::format(
                           "Terminating ReliableListener %1% on topic: %2%. "
                           "Reason: Unhandled exception while calling the "
                           "function set by "
                           "ReliableListener::terminate_on_exception. %3%") %
                         id_ % name_ % t.what()));
                return true;
            }
        }

    private:
        Listener listener_;
        int id_;
        std::shared_ptr<ringbuffer> ringbuffer_;
        int64_t sequence_;
        std::atomic<bool> cancelled_;
        logger& logger_;
        const std::string& name_;
        std::shared_ptr<spi::impl::ClientExecutionServiceImpl>
          execution_service_;        
        util::hz_thread_pool& executor_;
        serialization::pimpl::SerializationService& serialization_service_;
        int batch_size_;
        util::SynchronizedMap<int, util::concurrent::Cancellable>& runners_map_;
        std::shared_ptr<reliable_topic> topic_;
    };

    util::SynchronizedMap<int, util::concurrent::Cancellable> runners_map_;
    std::atomic<int> runner_counter_{ 0 };
    std::shared_ptr<spi::impl::ClientExecutionServiceImpl> execution_service_;
    util::hz_thread_pool& executor_;
    logger& logger_;
    int batch_size_;
    boost::shared_future<std::shared_ptr<ringbuffer>> ringbuffer_;
};
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
