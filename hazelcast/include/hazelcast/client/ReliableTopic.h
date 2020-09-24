/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/Ringbuffer.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/concurrent/Cancellable.h"

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
        class HAZELCAST_API ReliableTopic : public proxy::ReliableTopicImpl {
            friend class spi::ProxyManager;
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
                return proxy::ReliableTopicImpl::publish(toData(message));
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
            std::string addMessageListener(Listener &&listener) {
                int id = ++runner_counter_;
                std::shared_ptr<MessageRunner < Listener>>
                runner(new MessageRunner<Listener>(id, std::forward<Listener>(listener), ringbuffer, getName(),
                                                   getSerializationService(), *config, logger));
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
            bool removeMessageListener(const std::string &registrationId) {
                int id = util::IOUtil::to_value<int>(registrationId);
                auto runner = runners_map_.get(id);
                if (!runner) {
                    return false;
                }
                runner->cancel();
                runners_map_.remove(id);
                return true;
            };
        protected:
            void onDestroy() override {
                // cancel all runners
                for (auto &entry : runners_map_.clear()) {
                    entry.second->cancel();
                }

                // destroy the underlying ringbuffer
                ringbuffer->destroy();
            }

        private:
            ReliableTopic(const std::string &instanceName, spi::ClientContext *context) : proxy::ReliableTopicImpl(
                    instanceName, context) {}

            template<typename Listener>
            class MessageRunner
                    : public ExecutionCallback<ringbuffer::ReadResultSet>,
                      public std::enable_shared_from_this<MessageRunner<Listener>>,
                      public util::concurrent::Cancellable {
            public:
                MessageRunner(int id, Listener &&listener, const std::shared_ptr<Ringbuffer> &rb,
                              const std::string &topicName, serialization::pimpl::SerializationService &service,
                              const config::ReliableTopicConfig &reliableTopicConfig, util::ILogger &logger)
                        : listener_(listener), id_(id), ringbuffer_(rb), cancelled_(false), logger_(logger),
                        name_(topicName), executor_(rb, logger), serialization_service_(service),
                        config_(reliableTopicConfig) {
                    // we are going to listen to next publication. We don't care about what already has been published.
                    int64_t initialSequence = listener.retrieveInitialSequence();
                    if (initialSequence == -1) {
                        initialSequence = ringbuffer_->tailSequence().get() + 1;
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
                    m.maxCount = config_.getReadBatchSize();
                    executor_.execute(std::move(m));
                }

                // This method is called from the provided executor.
                void onResponse(const boost::optional<ringbuffer::ReadResultSet> &allMessages) override {
                    if (cancelled_) {
                        return;
                    }

                    // we process all messages in batch. So we don't release the thread and reschedule ourselves;
                    // but we'll process whatever was received in 1 go.
                    for (auto &item : allMessages->getItems()) {
                        try {
                            listener_.storeSequence(sequence_);
                            process(item.get<topic::impl::reliable::ReliableTopicMessage>().get_ptr());
                        } catch (exception::IException &e) {
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
                void onFailure(std::exception_ptr throwable) override {
                    if (cancelled_) {
                        return;
                    }

                    try {
                        std::rethrow_exception(throwable);
                    } catch (exception::IException &ie) {
                        int32_t err = ie.getErrorCode();
                        int32_t causeErrorCode = protocol::UNDEFINED;
                        try {
                            std::rethrow_if_nested(std::current_exception());
                        } catch (exception::IException &causeException) {
                            causeErrorCode = causeException.getErrorCode();
                        }
                        if (protocol::TIMEOUT == err) {
                            if (logger_.isFinestEnabled()) {
                                logger_.finest("MessageListener on topic: ", name_, " timed out. ",
                                              "Continuing from last known sequence: ", sequence_);
                            }
                            next();
                            return;
                        } else if (protocol::EXECUTION == err &&
                                   protocol::STALE_SEQUENCE == causeErrorCode) {
                            // StaleSequenceException.getHeadSeq() is not available on the client-side, see #7317
                            int64_t remoteHeadSeq = ringbuffer_->headSequence().get();

                            if (listener_.isLossTolerant()) {
                                if (logger_.isFinestEnabled()) {
                                    std::ostringstream out;
                                    out << "MessageListener " << id_ << " on topic: " << name_
                                        << " ran into a stale sequence. "
                                        << "Jumping from oldSequence: " << sequence_ << " to sequence: "
                                        << remoteHeadSeq;
                                    logger_.finest(out.str());
                                }
                                sequence_ = remoteHeadSeq;
                                next();
                                return;
                            }

                            std::ostringstream out;
                            out << "Terminating MessageListener:" << id_ << " on topic: " << name_
                                << "Reason: The listener "
                                   "was too slow or the retention period of the message has been violated. "
                                << "head: "
                                << remoteHeadSeq << " sequence:" << sequence_;
                            logger_.warning(out.str());
                        } else if (protocol::HAZELCAST_INSTANCE_NOT_ACTIVE == err) {
                            if (logger_.isFinestEnabled()) {
                                std::ostringstream out;
                                out << "Terminating MessageListener " << id_ << " on topic: " << name_ << ". "
                                    << " Reason: HazelcastInstance is shutting down";
                                logger_.finest(out.str());
                            }
                        } else if (protocol::DISTRIBUTED_OBJECT_DESTROYED == err) {
                            if (logger_.isFinestEnabled()) {
                                std::ostringstream out;
                                out << "Terminating MessageListener " << id_ << " on topic: " << name_
                                    << " Reason: Topic is destroyed";
                                logger_.finest(out.str());
                            }
                        } else {
                            std::ostringstream out;
                            out << "Terminating MessageListener " << id_ << " on topic: " << name_ << ". "
                                << " Reason: Unhandled exception, details:" << ie.what();
                            logger_.warning(out.str());
                        }

                        cancel();
                    }
                }

                bool cancel() override {
                    cancelled_.store(true);
                    return executor_.stop();
                }

                bool isCancelled() override {
                    return cancelled_.load();
                }
            private:
                void process(topic::impl::reliable::ReliableTopicMessage *message) {
                    //  proxy.localTopicStats.incrementReceives();
                    listener_.onMessage(toMessage(message));
                }

                topic::Message toMessage(topic::impl::reliable::ReliableTopicMessage *m) {
                    boost::optional<Member> member;
                    auto &addr = m->getPublisherAddress();
                    if (addr.has_value()) {
                        member = boost::make_optional<Member>(addr.value());
                    }
                    return topic::Message(name_, TypedData(std::move(m->getPayload()), serialization_service_),
                                          m->getPublishTime(), std::move(member));
                }

                bool terminate(const exception::IException &failure) {
                    if (cancelled_) {
                        return true;
                    }

                    try {
                        bool terminate = listener_.isTerminal(failure);
                        if (terminate) {
                            std::ostringstream out;
                            out << "Terminating MessageListener " << id_ << " on topic: " << name_ << ". "
                                << " Reason: Unhandled exception, details: " << failure.what();
                            logger_.warning(out.str());
                        } else {
                            if (logger_.isFinestEnabled()) {
                                std::ostringstream out;
                                out << "MessageListener " << id_ << " on topic: " << name_ << ". "
                                    << " ran into an exception, details:" << failure.what();
                                logger_.finest(out.str());
                            }
                        }
                        return terminate;
                    } catch (exception::IException &t) {
                        std::ostringstream out;
                        out << "Terminating MessageListener " << id_ << " on topic: " << name_ << ". "
                            << " Reason: Unhandled exception while calling ReliableMessageListener::isTerminal() method. "
                            << t.what();
                        logger_.warning(out.str());

                        return true;
                    }
                }

            private:
                Listener listener_;
                int id_;
                std::shared_ptr<Ringbuffer> ringbuffer_;
                int64_t sequence_;
                std::atomic<bool> cancelled_;
                util::ILogger &logger_;
                const std::string &name_;
                topic::impl::reliable::ReliableTopicExecutor executor_;
                serialization::pimpl::SerializationService &serialization_service_;
                const config::ReliableTopicConfig &config_;
            };

            util::SynchronizedMap<int, util::concurrent::Cancellable> runners_map_;
            std::atomic<int> runner_counter_{ 0 };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
