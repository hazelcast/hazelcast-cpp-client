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
//
// Created by ihsan demir on 27 May 2016.

#ifndef HAZELCAST_CLIENT_TOPIC_IMPL_RELIABLE_RELIABLETOPICEXECUTOR_H_
#define HAZELCAST_CLIENT_TOPIC_IMPL_RELIABLE_RELIABLETOPICEXECUTOR_H_

#include <memory>
#include <stdint.h>

#include "hazelcast/client/Ringbuffer.h"
#include "hazelcast/client/DataArray.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/client/ExecutionCallback.h"
#include "hazelcast/client/topic/impl/reliable/ReliableTopicMessage.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            namespace impl {
                namespace reliable {
                    class HAZELCAST_API ReliableTopicExecutor {
                    public:
                        enum MessageType {
                            GET_ONE_MESSAGE,
                            CANCEL
                        };

                        struct Message {
                            MessageType type;
                            int64_t sequence;
                            int32_t maxCount;
                            client::ExecutionCallback<DataArray<topic::impl::reliable::ReliableTopicMessage> > *callback;
                        };

                        ReliableTopicExecutor(Ringbuffer <ReliableTopicMessage> &rb, util::ILogger &logger);

                        virtual ~ReliableTopicExecutor();

                        /**
                         * Not thread safe method
                         */
                        void start();

                        void stop();

                        void execute(const Message &m);

                    private:
                        class Task {
                        public:
                            Task(Ringbuffer <ReliableTopicMessage> &rb, util::BlockingConcurrentQueue<Message> &q,
                                 util::AtomicBoolean &shutdown);

                            virtual void run();

                            virtual const std::string getName() const;

                        private:
                            Ringbuffer <ReliableTopicMessage> &rb;
                            util::BlockingConcurrentQueue<Message> &q;
                            util::AtomicBoolean &shutdown;
                        };

                        Ringbuffer <ReliableTopicMessage> &ringbuffer;
                        std::thread runnerThread;
                        util::BlockingConcurrentQueue<Message> q;
                        util::AtomicBoolean shutdown;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_TOPIC_IMPL_RELIABLE_RELIABLETOPICEXECUTOR_H_

