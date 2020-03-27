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
// Created by sancar koyunlu on 5/21/13.


#ifndef HAZELCAST_CONCURRENT_QUEUE
#define HAZELCAST_CONCURRENT_QUEUE

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/util/Mutex.h"
#include <deque>
#include <iostream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace util {
        template <typename T>
        /* Non blocking - synchronized queue - does not delete memory ever */
        class ConcurrentQueue {
        public:
            ConcurrentQueue() {

            }

            void offer(T *e) {
                util::LockGuard lg(m);
                internalQueue.push_back(e);
            }

            T *poll() {
                T *e = NULL;
                util::LockGuard lg(m);
                if (!internalQueue.empty()) {
                    e = internalQueue.front();
                    internalQueue.pop_front();
                }
                return e;
            }

            /**
             * Note that this method is not very efficient but it is only called very rarely when the connection is closed
             * Complexity: N2
             * @param itemToBeRemoved The item to be removed from the queue
             * @return number of items removed from the queue
             */
            int removeAll(const T *itemToBeRemoved) {
                util::LockGuard lg(m);
                int numErased = 0;
                bool isFound;
                do {
                    isFound = false;
                    for (typename std::deque<T *>::iterator it = internalQueue.begin();it != internalQueue.end(); ++it) {
                        T *e = *it;
                        if (itemToBeRemoved == e) {
                            internalQueue.erase(it);
                            isFound = true;
                            ++numErased;
                            break;
                        }
                    }
                } while (isFound);
                return numErased;
            }

        private:
            util::Mutex m;
            /**
             * Did not choose std::list which shall give better removeAll performance since deque is more efficient on
             * offer and poll due to data locality (best would be std::vector but it does not allow pop_front).
             */
            std::deque<T *> internalQueue;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_CONCURRENT_QUEUE

