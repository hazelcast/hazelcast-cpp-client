/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_UTIL_ATOMIC_H_
#define HAZELCAST_UTIL_ATOMIC_H_

#include <ostream>
#include <boost/noncopyable.hpp>

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace util {
        template<typename T>
        class Atomic : private boost::noncopyable {
        public:
            Atomic() {}

            Atomic(const T &v) : v(v) {
            }

            virtual ~Atomic() {
            }

            T operator--(int) {
                LockGuard lockGuard(mutex);
                return v--;
            }

            T operator-=(const T &delta) {
                LockGuard lockGuard(mutex);
                v -= delta;
                return v;
            }

            T operator++(int) {
                LockGuard lockGuard(mutex);
                return v++;
            }

            T operator++() {
                LockGuard lockGuard(mutex);
                return ++v;
            }

            T operator+=(const T &delta) {
                LockGuard lockGuard(mutex);
                v += delta;
                return v;
            }

            void operator=(const T &i) {
                LockGuard lockGuard(mutex);
                v = i;
            }

            void set(const T &i) {
                LockGuard lockGuard(mutex);
                v = i;
            }

            operator T() {
                return get();
            }

            T get() {
                LockGuard lockGuard(mutex);
                return v;
            }

            T operator--() {
                LockGuard lockGuard(mutex);
                return --v;
            }

            bool operator<=(const T &i) {
                LockGuard lockGuard(mutex);
                return v <= i;
            }

            bool operator==(const T &i) {
                LockGuard lockGuard(mutex);
                return i == v;
            }

            bool operator!=(const T &i) {
                LockGuard lockGuard(mutex);
                return i != v;
            }

            bool compareAndSet(const T &compareValue, const T &setValue) {
                LockGuard lockGuard(mutex);
                if(compareValue == v){
                    v = setValue;
                    return true;
                }
                return false;
            }

        protected:

            Mutex mutex;
            T v;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_Atomic

