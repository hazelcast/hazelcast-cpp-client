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

#include "hazelcast/util/HazelcastDll.h"

#include <mutex>
#include <deque>
#include <vector>
#include <iostream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        template <typename T>
        /* Non blocking - synchronized queue*/
        class SynchronizedQueue {
        public:
            void offer(const std::shared_ptr<T> &e) {
                std::lock_guard<std::mutex> lg(m);
                internalQueue.push_back(e);
            }

            std::shared_ptr<T> poll() {
                std::shared_ptr<T> e;
                std::lock_guard<std::mutex> lg(m);
                if (!internalQueue.empty()) {
                    e = internalQueue.front();
                    internalQueue.pop_front();
                }
                return e;
            }

            size_t size() {
                std::lock_guard<std::mutex> lg(m);
                return internalQueue.size();
            }

            std::vector<std::shared_ptr<T> > values() {
                std::lock_guard<std::mutex> lg(m);
                std::vector<std::shared_ptr<T> > values;
                for (typename std::deque<std::shared_ptr<T> >::const_iterator it = internalQueue.begin();
                     it != internalQueue.end(); ++it) {
                    values.push_back(*it);
                }
                return values;
            }

        private:
            std::mutex m;
            /**
             * Did not choose std::list which shall give better remove_all performance since deque is more efficient on
             * offer and poll due to data locality (best would be std::vector but it does not allow pop_front).
             */
            std::deque<std::shared_ptr<T> > internalQueue;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 



