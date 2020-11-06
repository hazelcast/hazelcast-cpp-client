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

#include <list>
#include <iostream>
#include <mutex>
#include <condition_variable>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/exception/InterruptedException.h"

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
            BlockingConcurrentQueue(size_t max_queue_capacity) : capacity_(max_queue_capacity), isInterrupted_(false) {
            }

            void push(const T &e) {
                std::unique_lock<std::mutex> lock(m_);
                while (internalQueue_.size() == capacity_) {
                    if (isInterrupted_) {
                        throw client::exception::InterruptedException("BlockingConcurrentQueue::push");
                    }
                    // wait on condition
                    notFull_.wait(lock);
                }
                internalQueue_.push_back(e);
                notEmpty_.notify_one();
            }

            T pop() {
                std::unique_lock<std::mutex> lock(m_);
                while (internalQueue_.empty()) {
                    if (isInterrupted_) {
                        throw client::exception::InterruptedException("BlockingConcurrentQueue::pop");
                    }
                    // wait for notEmpty condition
                    notEmpty_.wait(lock);
                }
                T element = internalQueue_.front();
                internalQueue_.pop_front();
                notFull_.notify_one();
                return element;
            }

            /**
             * Removes all of the elements from this collection (optional operation).
             * The collection will be empty after this method returns.
             *
             */
            void clear() {
                std::unique_lock<std::mutex> lock(m_);
                internalQueue_.clear();
                notFull_.notify_one();
            }

            void interrupt() {
                notFull_.notify_one();
                notEmpty_.notify_one();
                isInterrupted_ = true;
            }

            bool is_empty() {
                std::lock_guard<std::mutex> lock(m_);
                return internalQueue_.empty();
            }

            size_t size() {
                std::unique_lock<std::mutex> lock(m_);
                return internalQueue_.size();
            }

        private:
            std::mutex m_;
            /**
             * Did not choose std::list which shall give better remove_all performance since deque is more efficient on
             * offer and poll due to data locality (best would be std::vector but it does not allow pop_front).
             */
            std::list<T> internalQueue_;
            size_t capacity_;
            std::condition_variable notFull_;
            std::condition_variable notEmpty_;
            bool isInterrupted_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 



