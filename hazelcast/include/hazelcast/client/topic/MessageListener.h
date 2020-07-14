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

#include "hazelcast/client/topic/Message.h"
#include "hazelcast/client/serialization/serialization.h"

#include <memory>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            /**
             * Invoked when a message is received for the added topic. Note that topic guarantees message ordering.
             * Therefore there is only one thread invoking onMessage. The user should not keep the thread busy, but preferably
             * should dispatch it via an Executor. This will increase the performance of the topic.
             *
             * @param message the message that is received for the added topic
             */
            class HAZELCAST_API MessageListener {
            public:
                virtual ~MessageListener() = default;

                /**
                * Invoked when a message is received for the added topic. Note that topic guarantees message ordering.
                * Therefore there is only one thread invoking onMessage. The user should not keep the thread busy, but preferably
                * should dispatch it via an Executor. This will increase the performance of the topic.
                *
                * @param message the message that is received for the added topic
                */
                virtual void onMessage(Message &&message) = 0;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



