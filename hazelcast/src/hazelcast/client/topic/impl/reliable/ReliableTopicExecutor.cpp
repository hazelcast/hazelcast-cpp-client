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

#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"
#include "hazelcast/client/proxy/ClientRingbufferProxy.h"
#include "hazelcast/client/spi/impl/ClientInvocationFuture.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicExecutor::ReliableTopicExecutor(Ringbuffer<ReliableTopicMessage> &rb, util::ILogger &logger)
                    : ringbuffer(rb), runnerThread(boost::shared_ptr<util::Runnable>(new Task(ringbuffer, q, shutdown)), logger),
                      q(10), shutdown(false) {
                    }

                    ReliableTopicExecutor::~ReliableTopicExecutor() {
                        stop();
                    }

                    void ReliableTopicExecutor::start() {
                        runnerThread.start();
                    }

                    void ReliableTopicExecutor::stop() {
                        if (!shutdown.compareAndSet(false, true)) {
                            return;
                        }

                        topic::impl::reliable::ReliableTopicExecutor::Message m;
                        m.type = topic::impl::reliable::ReliableTopicExecutor::CANCEL;
                        m.callback = NULL;
                        m.sequence = -1;
                        execute(m);
                        runnerThread.join();
                    }

                    void ReliableTopicExecutor::execute(const Message &m) {
                        q.push(m);
                    }

                    void ReliableTopicExecutor::Task::run() {
                        while (!shutdown) {
                            Message m = q.pop();
                            if (CANCEL == m.type) {
                                // exit the thread
                                return;
                            }
                            try {
                                proxy::ClientRingbufferProxy<ReliableTopicMessage> &ringbuffer =
                                        static_cast<proxy::ClientRingbufferProxy<ReliableTopicMessage> &>(rb);
                                boost::shared_ptr<spi::impl::ClientInvocationFuture> future = ringbuffer.readManyAsync(m.sequence, 1, m.maxCount);
                                boost::shared_ptr<protocol::ClientMessage> responseMsg;
                                do {
                                    if (future->get(1000, TimeUnit::MILLISECONDS())) {
                                        responseMsg = future->get(); // every one second
                                    }
                                } while (!shutdown && (protocol::ClientMessage *)NULL == responseMsg.get());

                                if (!shutdown) {
                                    boost::shared_ptr<DataArray<ReliableTopicMessage> > allMessages(ringbuffer.getReadManyAsyncResponseObject(
                                            responseMsg));

                                    m.callback->onResponse(allMessages);
                                }
                            } catch (exception::IException &e) {
                                m.callback->onFailure(boost::shared_ptr<exception::IException>(e.clone()));
                            }
                        }

                    }

                    const std::string ReliableTopicExecutor::Task::getName() const {
                        return "ReliableTopicExecutor Task";
                    }

                    ReliableTopicExecutor::Task::Task(Ringbuffer<ReliableTopicMessage> &rb,
                                                      util::BlockingConcurrentQueue<ReliableTopicExecutor::Message> &q,
                                                      util::AtomicBoolean &shutdown) : rb(rb), q(q),
                                                                                       shutdown(shutdown) {}
                }
            }
        }
    }
}

