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

#include <ostream>
#include <mutex>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        template<typename T>
        class Sync {
        public:
            Sync() = default;

            Sync(const T &v) : v_(v) {
            }

            virtual ~Sync() = default;

            T operator--(int) {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return v_--;
            }

            T operator-=(const T &delta) {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                v_ -= delta;
                return v_;
            }

            T operator++(int) {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return v_++;
            }

            T operator++() {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return ++v_;
            }

            T operator+=(const T &delta) {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                v_ += delta;
                return v_;
            }

            void operator=(const T &i) {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                v_ = i;
            }

            void set(const T &i) {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                v_ = i;
            }

            operator T() const {
                return get();
            }

            T get() const {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return v_;
            }

            T operator--() {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return --v_;
            }

            bool operator<=(const T &i) const {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return v_ <= i;
            }

            bool operator==(const T &i) const {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return i == v_;
            }

            bool operator!=(const T &i) const {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                return i != v_;
            }

            bool compare_and_set(const T &compare_value, const T &set_value) {
                std::lock_guard<std::mutex> lockGuard(mutex_);
                if(compare_value == v_){
                    v_ = set_value;
                    return true;
                }
                return false;
            }

        protected:
            mutable std::mutex mutex_;
            T v_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



