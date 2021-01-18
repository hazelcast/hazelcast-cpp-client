/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/export.h"
#include "hazelcast/client/exception/protocol_exceptions.h"

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
            BlockingConcurrentQueue(size_t max_queue_capacity) : capacity_(max_queue_capacity), is_interrupted_(false) {
            }

            void push(const T &e) {
                std::unique_lock<std::mutex> lock(m_);
                while (internal_queue_.size() == capacity_) {
                    if (is_interrupted_) {
                        throw client::exception::interrupted("BlockingConcurrentQueue::push");
                    }
                    // wait on condition
                    not_full_.wait(lock);
                }
                internal_queue_.push_back(e);
                not_empty_.notify_one();
            }

            T pop() {
                std::unique_lock<std::mutex> lock(m_);
                while (internal_queue_.empty()) {
                    if (is_interrupted_) {
                        throw client::exception::interrupted("BlockingConcurrentQueue::pop");
                    }
                    // wait for notEmpty condition
                    not_empty_.wait(lock);
                }
                T element = internal_queue_.front();
                internal_queue_.pop_front();
                not_full_.notify_one();
                return element;
            }

            /**
             * Removes all of the elements from this collection (optional operation).
             * The collection will be empty after this method returns.
             *
             */
            void clear() {
                std::unique_lock<std::mutex> lock(m_);
                internal_queue_.clear();
                not_full_.notify_one();
            }

            void interrupt() {
                not_full_.notify_one();
                not_empty_.notify_one();
                is_interrupted_ = true;
            }

            bool is_empty() {
                std::lock_guard<std::mutex> lock(m_);
                return internal_queue_.empty();
            }

            size_t size() {
                std::unique_lock<std::mutex> lock(m_);
                return internal_queue_.size();
            }

        private:
            std::mutex m_;
            /**
             * Did not choose std::list which shall give better remove_all performance since deque is more efficient on
             * offer and poll due to data locality (best would be std::vector but it does not allow pop_front).
             */
            std::list<T> internal_queue_;
            size_t capacity_;
            std::condition_variable not_full_;
            std::condition_variable not_empty_;
            bool is_interrupted_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 



