/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/RingbufferImpl.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicExecutor.h"

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                namespace reliable {
                    ReliableTopicExecutor::ReliableTopicExecutor(Ringbuffer<topic::impl::reliable::ReliableTopicMessage> *rb)
                    : ringbuffer(rb), q(10), shutdown(false) {
                    }

                    ReliableTopicExecutor::~ReliableTopicExecutor() {
                        stop();
                    }

                    void ReliableTopicExecutor::start() {
                        if (NULL == runnerThread.get()) {
                            runnerThread = std::auto_ptr<util::Thread>(new util::Thread(executerRun, &shutdown, &q, ringbuffer));
                        }
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

                        /**
                         * The following line is commented out due to bug
                         * https://github.com/hazelcast/hazelcast-cpp-client/issues/339
                         */
                        //runnerThread->cancel();
                        runnerThread->join();
                        runnerThread.reset();
                    }

                    void ReliableTopicExecutor::execute(const Message &m) {
                        q.push(m);
                    }

                    void ReliableTopicExecutor::executerRun(util::ThreadArgs &args) {
                        util::AtomicBoolean *shutdownFlag = (util::AtomicBoolean *)args.arg0;
                        util::BlockingConcurrentQueue<Message> *q = (util::BlockingConcurrentQueue<Message> *)args.arg1;
                        Ringbuffer<topic::impl::reliable::ReliableTopicMessage> * rb = (Ringbuffer<topic::impl::reliable::ReliableTopicMessage> *)args.arg2;

                        while (!(*shutdownFlag)) {
                            Message m = q->pop();
                            if (CANCEL == m.type) {
                                // exit the thread
                                return;
                            }
                            try {
                                proxy::RingbufferImpl<topic::impl::reliable::ReliableTopicMessage> *ringbuffer =
                                        (proxy::RingbufferImpl<topic::impl::reliable::ReliableTopicMessage> *)rb;

                                connection::CallFuture future = ringbuffer->readManyAsync(m.sequence, 1, m.maxCount);
                                std::auto_ptr<protocol::ClientMessage> responseMsg;
                                do {
                                    if (future.waitFor(1000)) {
                                        responseMsg = future.get(); // every one second
                                    }
                                } while (!(*shutdownFlag) && (protocol::ClientMessage *)NULL == responseMsg.get());

                                if (!(*shutdownFlag)) {
                                    std::auto_ptr<DataArray<ReliableTopicMessage> > allMessages = ringbuffer->getReadManyAsyncResponseObject(
                                            responseMsg);

                                    m.callback->onResponse(allMessages.get());
                                }
                            } catch (exception::ProtocolException &e) {
                                m.callback->onFailure(&e);
                            }
                        }
                    }
                }
            }
        }
    }
}

