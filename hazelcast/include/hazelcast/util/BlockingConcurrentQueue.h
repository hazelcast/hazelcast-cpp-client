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
//
// Created by ihsan demir on 02 June 2016.


#ifndef HAZELCAST_UTIL_BLOCKINGCONCURRENTQUEUE_H_
#define HAZELCAST_UTIL_BLOCKINGCONCURRENTQUEUE_H_

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/ConditionVariable.h"
#include <list>
#include <iostream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace util {
        template <typename T>
        /* Blocking - synchronized queue */
        class BlockingConcurrentQueue {
        public:
            BlockingConcurrentQueue(size_t maxQueueCapacity) : capacity(maxQueueCapacity) {
            }

            void push(const T &e) {
                util::LockGuard lg(m);
                while (internalQueue.size() == capacity) {
                    // wait on condition
                    notFull.wait(m);
                }
                internalQueue.push_back(e);
                notEmpty.notify();
            }

            T pop() {
                util::LockGuard lg(m);
                while (internalQueue.empty()) {
                    // wait for notEmpty condition
                    notEmpty.wait(m);
                }
                T element = internalQueue.front();
                internalQueue.pop_front();
                notFull.notify();
                return element;
            }

        private:
            util::Mutex m;
            /**
             * Did not choose std::list which shall give better removeAll performance since deque is more efficient on
             * offer and poll due to data locality (best would be std::vector but it does not allow pop_front).
             */
            std::list<T> internalQueue;
            size_t capacity;
            util::ConditionVariable notFull;
            util::ConditionVariable notEmpty;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_UTIL_BLOCKINGCONCURRENTQUEUE_H_

