/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan demir on Nov 12, 2015.
//


#ifndef HAZELCAST_CLIENT_ATOMIC_H_
#define HAZELCAST_CLIENT_ATOMIC_H_

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        template<typename T>
        class Atomic {
        public:
            Atomic() {
            }

            Atomic(T v) : v(v) {
            }

            T operator--(int) {
                LockGuard lockGuard(mutex);
                return v--;
            }

            T operator++(int) {
                LockGuard lockGuard(mutex);
                return v++;
            }

            T operator++() {
                LockGuard lockGuard(mutex);
                return ++v;
            }

            void operator=(T i) {
                LockGuard lockGuard(mutex);
                v = i;
            }

            operator T() {
                LockGuard lockGuard(mutex);
                return v;
            }

            T operator--() {
                LockGuard lockGuard(mutex);
                return --v;
            }

            bool operator<=(T i) {
                LockGuard lockGuard(mutex);
                return v <= i;
            }

            bool operator==(T i) {
                LockGuard lockGuard(mutex);
                return i == v;
            }

            bool operator!=(T i) {
                LockGuard lockGuard(mutex);
                return i != v;
            }

        private:
            Mutex mutex;
            T v;

            Atomic(const Atomic &rhs);

            void operator=(const Atomic &rhs);
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Atomic

