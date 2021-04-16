/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/ReliableTopicImpl.h"
#include "hazelcast/client/ringbuffer.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/concurrent/Cancellable.h"
#include "hazelcast/logger.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicMessage.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {

        /**
        * Hazelcast provides distribution mechanism for publishing messages that are delivered to multiple subscribers
        * which is also known as publish/subscribe (pub/sub) messaging model. Publish and subscriptions are cluster-wide.
        * When a member subscribes for a topic, it is actually registering for messages published by any member in the cluster,
        * including the new members joined after you added the listener.
        *
        * Messages are ordered, meaning, listeners(subscribers)
        * will process the messages in the order they are actually published. If cluster member M publishes messages
        * m1, m2, m3...mn to a topic T, then Hazelcast makes sure that all of the subscribers of topic T will receive
        * and process m1, m2, m3...mn in order.
        *
        */
        class HAZELCAST_API reliable_topic : public proxy::ReliableTopicImpl {
            friend class spi::ProxyManager;
            friend class hazelcast_client;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:topicService";

            /**
            * Publishes the message to all subscribers of this topic
            * Current implementation only supports DISCARD_OLDEST policy as in Java client. The other policies will be
            * available when async API is completed.  Using this policy the oldest item is overwritten no matter it is
            * not old enough to retire.
            *
            * @param message The message to be published
            */
            template<typename E>
            boost::future<void> publish(const E &message) {
                return proxy::ReliableTopicImpl::publish(to_data(message));
            }

            /**
            * Subscribes to this topic. When someone publishes a message on this topic.
            * onMessage() function of the given MessageListener is called. More than one message listener can be
            * added on one instance.
            *
            * Warning 1: If listener should do a time consuming operation, off-load the operation to another thread.
            * otherwise it will slow down the system.
            *
            * Warning 2: Do not make a call to hazelcast. It can cause deadlock.
            *
            * Warning 3: Make sure that the MessageListener object is not destroyed until the removeListener is called,
            * since the library will use the MessageListener reference to deliver incoming messages.
            *
            * @param listener the MessageListener to add.
            *
            * @return returns registration id.
            */
            template<typename Listener>
            std::string add_message_listener(Listener &&listener) {
                int id = ++runner_counter_;
                std::shared_ptr<MessageRunner < Listener>>
                runner(new MessageRunner<Listener>(id, std::forward<Listener>(listener), ringbuffer_, get_name(),
                                                   get_serialization_service(), batch_size_, logger_));
                runners_map_.put(id, runner);
                runner->next();
                return std::to_string(id);
            }

            /**
            * Stops receiving messages for the given message listener. If the given listener already removed,
            * this method does nothing.
            *
            * @param registrationId Id of listener registration.
            *
            * @return true if registration is removed, false otherwise
            */
            bool remove_message_listener(const std::string &registration_id);
        protected:
            void on_destroy() override;

        private:
            reliable_topic(std::shared_ptr<ringbuffer> rb, const std::string &instance_name,
                           spi::ClientContext *context);

            template<typename Listener>
            class MessageRunner
                    : public execution_callback<rb::read_result_set>,
                      public std::enable_shared_from_this<MessageRunner<Listener>>,
                      public util::concurrent::Cancellable {
            public:
                MessageRunner(int id, Listener &&listener, const std::shared_ptr<ringbuffer> &rb,
                              const std::string &topic_name, serialization::pimpl::SerializationService &service,
                              int batch_size, logger &lg)
                        : listener_(listener), id_(id), ringbuffer_(rb), cancelled_(false), logger_(lg),
                        name_(topic_name), executor_(rb, lg), serialization_service_(service),
                        batch_size_(batch_size) {
                    // we are going to listen to next publication. We don't care about what already has been published.
                    int64_t initialSequence = listener.initial_sequence_id_;
                    if (initialSequence == -1) {
                        initialSequence = ringbuffer_->tail_sequence().get() + 1;
                    }
                    sequence_ = initialSequence;
                }

                ~MessageRunner() override = default;

                void next() {
                    if (cancelled_) {
                        return;
                    }

                    topic::impl::reliable::ReliableTopicExecutor::Message m;
                    m.type = topic::impl::reliable::ReliableTopicExecutor::GET_ONE_MESSAGE;
                    m.callback = this->shared_from_this();
                    m.sequence = sequence_;
                    m.max_count = batch_size_;
                    executor_.execute(std::move(m));
                }

                // This method is called from the provided executor.
                void on_response(const boost::optional<rb::read_result_set> &all_messages) override {
                    if (cancelled_) {
                        return;
                    }

                    // we process all messages in batch. So we don't release the thread and reschedule ourselves;
                    // but we'll process whatever was received in 1 go.
                    for (auto &item : all_messages->get_items()) {
                        try {
                            listener_.store_sequence_id_(sequence_);
                            process(item.get<topic::impl::reliable::ReliableTopicMessage>().get_ptr());
                        } catch (exception::iexception &e) {
                            if (terminate(e)) {
                                cancel();
                                return;
                            }
                        }

                        sequence_++;
                    }

                    next();
                }

                // This method is called from the provided executor.
                void on_failure(std::exception_ptr throwable) override {
                    if (cancelled_) {
                        return;
                    }

                    try {
                        std::rethrow_exception(throwable);
                    } catch (exception::iexception &ie) {
                        int32_t err = ie.get_error_code();
                        int32_t causeErrorCode = protocol::UNDEFINED;
                        try {
                            std::rethrow_if_nested(ie);
                        } catch (exception::iexception &causeException) {
                            causeErrorCode = causeException.get_error_code();
                        }
                        if (protocol::TIMEOUT == err) {
                            HZ_LOG(logger_, finest, 
                                boost::str(boost::format("MessageListener on topic: %1% timed out. "
                                                         "Continuing from last known sequence: %2%")
                                                         % name_ % sequence_)
                            );
                            next();
                            return;
                        } else if (protocol::EXECUTION == err &&
                                   protocol::STALE_SEQUENCE == causeErrorCode) {
                            // stale_sequence_exception.getHeadSeq() is not available on the client-side, see #7317
                            int64_t remoteHeadSeq = ringbuffer_->head_sequence().get();

                            if (listener_.loss_tolerant_) {
                                HZ_LOG(logger_, finest, 
                                    boost::str(boost::format("MessageListener %1% on topic: %2% "
                                                             "ran into a stale sequence. "
                                                             "Jumping from old sequence: %3% "
                                                             "to sequence: %4%")
                                                             % id_ % name_ % sequence_ % remoteHeadSeq)
                                );
                                sequence_ = remoteHeadSeq;
                                next();
                                return;
                            }

                            HZ_LOG(logger_, warning,
                                boost::str(boost::format("Terminating MessageListener: %1% on topic: %2%"
                                                         "Reason: The listener was too slow or the retention "
                                                         "period of the message has been violated. "
                                                         "head: %3% sequence: %4%")
                                                         % id_ % name_ % remoteHeadSeq % sequence_)
                            );
                        } else if (protocol::HAZELCAST_INSTANCE_NOT_ACTIVE == err) {
                            HZ_LOG(logger_, finest, 
                                boost::str(boost::format("Terminating MessageListener %1% on topic: %2%. "
                                                         "Reason: HazelcastInstance is shutting down")
                                                         % id_ % name_)
                            );
                        } else if (protocol::DISTRIBUTED_OBJECT_DESTROYED == err) {
                            HZ_LOG(logger_, finest, 
                                boost::str(boost::format("Terminating MessageListener %1% on topic: %2%. "
                                                         "Reason: Topic is destroyed")
                                                         % id_ % name_)
                            );
                        } else {
                            HZ_LOG(logger_, warning, 
                                boost::str(boost::format("Terminating MessageListener %1% on topic: %2%. "
                                                         "Reason: Unhandled exception, details: %3%")
                                                         % id_ % name_ % ie.what())
                            );
                        }

                        cancel();
                    }
                }

                bool cancel() override {
                    cancelled_.store(true);
                    return executor_.stop();
                }

                bool is_cancelled() override {
                    return cancelled_.load();
                }
            private:
                void process(topic::impl::reliable::ReliableTopicMessage *message) {
                    //  proxy.localTopicStats.incrementReceives();
                    listener_.received_(to_message(message));
                }

                topic::message to_message(topic::impl::reliable::ReliableTopicMessage *message) {
                    boost::optional<member> m;
                    auto &addr = message->get_publisher_address();
                    if (addr.has_value()) {
                        m = boost::make_optional<member>(addr.value());
                    }
                    return topic::message(name_, typed_data(std::move(message->get_payload()), serialization_service_),
                                          message->get_publish_time(), std::move(m));
                }

                bool terminate(const exception::iexception &failure) {
                    if (cancelled_) {
                        return true;
                    }

                    try {
                        bool terminate = listener_.terminal_(failure);
                        if (terminate) {
                            HZ_LOG(logger_, warning,
                                boost::str(boost::format("Terminating ReliableListener %1% "
                                                         "on topic: %2%. "
                                                         "Reason: Unhandled exception, details: %3%")
                                                         % id_ % name_ % failure.what())
                            );
                        } else {
                            HZ_LOG(logger_, finest,
                                boost::str(boost::format("ReliableListener %1% on topic: %2% "
                                                         "ran into an exception, details: %3%")
                                                         % id_ % name_ % failure.what())
                            );
                        }
                        return terminate;
                    } catch (exception::iexception &t) {
                        HZ_LOG(logger_, warning,
                            boost::str(boost::format("Terminating ReliableListener %1% on topic: %2%. "
                                                     "Reason: Unhandled exception while calling the function set by "
                                                     "ReliableListener::terminate_on_exception. %3%")
                                                     % id_ % name_ % t.what())
                        );
                        return true;
                    }
                }

            private:
                Listener listener_;
                int id_;
                std::shared_ptr<ringbuffer> ringbuffer_;
                int64_t sequence_;
                std::atomic<bool> cancelled_;
                logger &logger_;
                const std::string &name_;
                topic::impl::reliable::ReliableTopicExecutor executor_;
                serialization::pimpl::SerializationService &serialization_service_;
                int batch_size_;
            };

            util::SynchronizedMap<int, util::concurrent::Cancellable> runners_map_;
            std::atomic<int> runner_counter_{ 0 };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
