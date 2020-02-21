/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by ihsan demir on 31 May 2016.
#ifndef HAZELCAST_CLIENT_RELIABLETOPIC_H_
#define HAZELCAST_CLIENT_RELIABLETOPIC_H_

#include "hazelcast/client/proxy/ReliableTopicImpl.h"
#include "hazelcast/client/Ringbuffer.h"
#include "hazelcast/client/DataArray.h"
#include "hazelcast/client/topic/impl/TopicEventHandlerImpl.h"

#include <string>
#include <memory>

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
        template<typename E>
        class ReliableTopic : public proxy::ReliableTopicImpl {
            friend class impl::HazelcastClientInstanceImpl;

        public:

            /**
            * Publishes the message to all subscribers of this topic
            * Current implementation only supports DISCARD_OLDEST policy as in Java client. The other policies will be
            * available when async API is completed.  Using this policy the oldest item is overwritten no matter it is
            * not old enough to retire.
            *
            * @param message The message to be published
            */
            void publish(const E *message) {
                serialization::pimpl::Data data = getContext().getSerializationService().template toData<E>(message);
                proxy::ReliableTopicImpl::publish(data);
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
            std::string addMessageListener(topic::ReliableMessageListener<E> &listener) {
                int id = ++runnerCounter;
                std::shared_ptr<MessageRunner < E> >
                runner(new MessageRunner<E>(id, &listener, ringbuffer.get(), getName(),
                                            &getContext().getSerializationService(), config, logger));
                runnersMap.put(id, runner);
                runner->next();
                return util::IOUtil::to_string<int>(id);
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
                std::shared_ptr<MessageRunner < E> > runner = runnersMap.get(id);
                if (NULL == runner) {
                    return false;
                }
                runner->cancel();
                runnersMap.remove(id);
                return true;
            };
        protected:
            virtual void onDestroy() {
                // cancel all runners
                std::vector<std::pair<int, std::shared_ptr<MessageRunner < E> > > > runners = runnersMap.clear();
                for (typename std::vector<std::pair<int, std::shared_ptr<MessageRunner < E> > > >
                     ::const_iterator it = runners.begin();it != runners.end();++it) {
                    it->second->cancel();
                }

                // destroy the underlying ringbuffer
                ringbuffer->destroy();
            }

        private:
            ReliableTopic(const std::string &instanceName, spi::ClientContext *context,
                          std::shared_ptr<Ringbuffer<topic::impl::reliable::ReliableTopicMessage> > rb)
                    : proxy::ReliableTopicImpl(instanceName, context, rb) {
            }

            template<typename T>
            class MessageRunner
                    : ExecutionCallback<DataArray<topic::impl::reliable::ReliableTopicMessage> > {
            public:
                MessageRunner(int id, topic::ReliableMessageListener<T> *listener,
                              Ringbuffer<topic::impl::reliable::ReliableTopicMessage> *rb,
                              const std::string &topicName, serialization::pimpl::SerializationService *service,
                              const config::ReliableTopicConfig *reliableTopicConfig, util::ILogger &logger)
                        : cancelled(false), logger(logger), name(topicName), executor(*rb, logger),
                          serializationService(service), config(reliableTopicConfig) {
                    this->id = id;
                    this->listener = listener;
                    this->ringbuffer = rb;

                    // we are going to listen to next publication. We don't care about what already has been published.
                    int64_t initialSequence = listener->retrieveInitialSequence();
                    if (initialSequence == -1) {
                        initialSequence = ringbuffer->tailSequence() + 1;
                    }
                    this->sequence = initialSequence;

                    this->executor.start();
                }


                virtual ~MessageRunner() {}

                void next() {
                    if (cancelled) {
                        return;
                    }

                    topic::impl::reliable::ReliableTopicExecutor::Message m;
                    m.type = topic::impl::reliable::ReliableTopicExecutor::GET_ONE_MESSAGE;
                    m.callback = this;
                    m.sequence = sequence;
                    m.maxCount = config->getReadBatchSize();
                    executor.execute(m);
                }

                // This method is called from the provided executor.
                void onResponse(
                        const std::shared_ptr<DataArray<topic::impl::reliable::ReliableTopicMessage> > &allMessages) {
                    if (cancelled) {
                        return;
                    }

                    size_t numMessages = allMessages->size();

                    // we process all messages in batch. So we don't release the thread and reschedule ourselves;
                    // but we'll process whatever was received in 1 go.
                    for (size_t i = 0; i < numMessages; ++i) {
                        try {
                            listener->storeSequence(sequence);
                            process(allMessages->get(i));
                        } catch (exception::IException &e) {
                            if (terminate(e)) {
                                cancel();
                                return;
                            }
                        }

                        sequence++;
                    }

                    next();
                }

                // This method is called from the provided executor.
                void onFailure(const std::shared_ptr<exception::IException> &throwable) {
                    if (cancelled) {
                        return;
                    }

                    int32_t err = throwable->getErrorCode();
                    if (protocol::TIMEOUT == err) {
                        if (logger.isFinestEnabled()) {
                            logger.finest() << "MessageListener " << listener << " on topic: " << name << " timed out. "
                                          << "Continuing from last known sequence: " << sequence;
                        }
                        next();
                        return;
                    } else if (protocol::EXECUTION == err &&
                        protocol::STALE_SEQUENCE == throwable->getCauseErrorCode()) {
                        // StaleSequenceException.getHeadSeq() is not available on the client-side, see #7317
                        int64_t remoteHeadSeq = ringbuffer->headSequence();

                        if (listener->isLossTolerant()) {
                            if (logger.isFinestEnabled()) {
                                std::ostringstream out;
                                out << "MessageListener " << id << " on topic: " << name
                                    << " ran into a stale sequence. "
                                    << "Jumping from oldSequence: " << sequence << " to sequence: " << remoteHeadSeq;
                                logger.finest(out.str());
                            }
                            sequence = remoteHeadSeq;
                            next();
                            return;
                        }

                        std::ostringstream out;
                        out << "Terminating MessageListener:" << id << " on topic: " << name << "Reason: The listener "
                                                                                                "was too slow or the retention period of the message has been violated. "
                            << "head: "
                            << remoteHeadSeq << " sequence:" << sequence;
                        logger.warning(out.str());
                    } else if (protocol::HAZELCAST_INSTANCE_NOT_ACTIVE == err) {
                        if (logger.isFinestEnabled()) {
                            std::ostringstream out;
                            out << "Terminating MessageListener " << id << " on topic: " << name << ". "
                                << " Reason: HazelcastInstance is shutting down";
                            logger.finest(out.str());
                        }
                    } else if (protocol::DISTRIBUTED_OBJECT_DESTROYED == err) {
                        if (logger.isFinestEnabled()) {
                            std::ostringstream out;
                            out << "Terminating MessageListener " << id << " on topic: " << name
                                << " Reason: Topic is destroyed";
                            logger.finest(out.str());
                        }
                    } else {
                        std::ostringstream out;
                        out << "Terminating MessageListener " << id << " on topic: " << name << ". "
                            << " Reason: Unhandled exception, details:" << throwable->what();
                        logger.warning(out.str());
                    }

                    cancel();
                }

                void cancel() {
                    cancelled.store(true);
                    executor.stop();
                }

            private:
                void process(const topic::impl::reliable::ReliableTopicMessage *message) {
                    //  proxy.localTopicStats.incrementReceives();
                    listener->onMessage(std::move(toMessage(message)));
                }

                std::unique_ptr<topic::Message<T> > toMessage(const topic::impl::reliable::ReliableTopicMessage *m) {
                    std::shared_ptr<Member> member;
                    const Address *addr = m->getPublisherAddress();
                    if (addr != NULL) {
                        member = std::shared_ptr<Member>(new Member(*addr));
                    }
                    std::unique_ptr<T> msg = serializationService->toObject<T>(m->getPayload());
                    return std::unique_ptr<topic::Message<T> >(
                            new topic::impl::MessageImpl<T>(name, msg, m->getPublishTime(), member));
                }

                bool terminate(const exception::IException &failure) {
                    if (cancelled) {
                        return true;
                    }

                    try {
                        bool terminate = listener->isTerminal(failure);
                        if (terminate) {
                            std::ostringstream out;
                            out << "Terminating MessageListener " << id << " on topic: " << name << ". "
                                << " Reason: Unhandled exception, details: " << failure.what();
                            logger.warning(out.str());
                        } else {
                            if (logger.isFinestEnabled()) {
                                std::ostringstream out;
                                out << "MessageListener " << id << " on topic: " << name << ". "
                                    << " ran into an exception, details:" << failure.what();
                                logger.finest(out.str());
                            }
                        }
                        return terminate;
                    } catch (exception::IException &t) {
                        std::ostringstream out;
                        out << "Terminating MessageListener " << id << " on topic: " << name << ". "
                            << " Reason: Unhandled exception while calling ReliableMessageListener::isTerminal() method. "
                            << t.what();
                        logger.warning(out.str());

                        return true;
                    }
                }

            private:
                topic::ReliableMessageListener<T> *listener;
                int id;
                Ringbuffer<topic::impl::reliable::ReliableTopicMessage> *ringbuffer;
                int64_t sequence;
                util::AtomicBoolean cancelled;
                util::ILogger &logger;
                const std::string &name;
                topic::impl::reliable::ReliableTopicExecutor executor;
                serialization::pimpl::SerializationService *serializationService;
                const config::ReliableTopicConfig *config;
            };

            util::SynchronizedMap<int, MessageRunner<E> > runnersMap;
            util::AtomicInt runnerCounter;
        };
    }
}

#endif //HAZELCAST_CLIENT_RELIABLETOPIC_H_

