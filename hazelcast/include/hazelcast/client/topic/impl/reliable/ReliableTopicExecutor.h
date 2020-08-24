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

#include <memory>
#include <atomic>
#include <stdint.h>

#include "hazelcast/client/Ringbuffer.h"
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
                            std::shared_ptr<ExecutionCallback<ringbuffer::ReadResultSet>> callback;
                        };

                        ReliableTopicExecutor(std::shared_ptr<Ringbuffer> rb, util::ILogger &logger);

                        virtual ~ReliableTopicExecutor();

                        /**
                         * Not thread safe method
                         */
                        void start();

                        bool stop();

                        void execute(Message m);

                    private:
                        class Task {
                        public:
                            Task(std::shared_ptr<Ringbuffer> rb, util::BlockingConcurrentQueue<Message> &q,
                                 std::atomic<bool> &shutdown);

                            virtual void run();

                            virtual std::string getName() const;

                        private:
                            std::shared_ptr<Ringbuffer> rb;
                            util::BlockingConcurrentQueue<Message> &q;
                            std::atomic<bool> &shutdown;
                        };

                        std::shared_ptr<Ringbuffer> ringbuffer;
                        std::thread runnerThread;
                        util::BlockingConcurrentQueue<Message> q;
                        std::atomic<bool> shutdown;
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



