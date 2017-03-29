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
#ifndef HAZELCAST_UTIL_SHARED_H_
#define HAZELCAST_UTIL_SHARED_H_

#include <ostream>
#include <cassert>

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        template<typename T>
        class SharedPtr {
        public:
            SharedPtr() : ptr(NULL), count(new int(1)), mutex(new util::Mutex()) {
            }

            SharedPtr(T *ptr) : ptr(ptr), count(new int(1)), mutex(new util::Mutex()) {
            }

            SharedPtr(std::auto_ptr<T> ptr) : ptr(ptr.release()), count(new int(1)), mutex(new util::Mutex()) {
            }

            virtual ~SharedPtr() {
                //if reset or assignment done, before  destructor
                //it is possible that count is already at 0
                //decrement will have updated count as -1 which is noop for it.
                decrement();
            }

            SharedPtr(const SharedPtr<T> &rhs) {
                LockGuard lockGuard(*(rhs.mutex));
                (*(rhs.count))++;

                ptr = rhs.ptr;
                count = rhs.count;
                mutex = rhs.mutex;
            }

            void operator=(const SharedPtr<T> &rhs) {
                if (ptr != rhs.get()) {
                    decrement();
                }
                LockGuard lockGuard(*(rhs.mutex));
                (*(rhs.count))++;

                ptr = rhs.ptr;
                count = rhs.count;
                mutex = rhs.mutex;
            }

            void reset() {
                decrement();
                ptr = NULL;
                count = new int(1);
                mutex = new util::Mutex();
            }

            void reset(T* rhsPtr) {
                decrement();
                ptr = rhsPtr;
                count = new int(1);
                mutex = new util::Mutex();
            }

            template<typename X>
            SharedPtr<X> castShare() const {
                LockGuard lockGuard(*(mutex));
                (*(count))++;
                return SharedPtr<X>(static_cast<X *>(ptr), count, mutex);
            }

            T *get() const {
                return ptr;
            }

            T *get() {
                return ptr;
            }

            T *operator->() const {
                return ptr;
            }

            T *operator->() {
                return ptr;
            }

            T &operator*() const {
                return *ptr;
            }

            T &operator*() {
                return *ptr;
            }

            int getCount() {
                LockGuard lockGuard(*mutex);
                int c = *count;
                return c;
            }

            bool operator<(const hazelcast::util::SharedPtr<T> &rhs) const{
                if(ptr == NULL || rhs.ptr == NULL) return false;
                return *ptr < *(rhs.ptr);
            }

            bool operator==(const hazelcast::util::SharedPtr<T> &rhs) const{
                if(ptr == NULL && rhs.ptr == NULL) return true;
                if(ptr == NULL || rhs.ptr == NULL) return false;
                return *ptr == *(rhs.ptr);
            }

            bool operator!=(const hazelcast::util::SharedPtr<T> &rhs) const{
                return !(*ptr == *(rhs.ptr));
            }

            SharedPtr(T *ptr, int *count, util::Mutex *mutex) : ptr(ptr), count(count), mutex(mutex) {
            }

        private:
            void decrement() {
                mutex->lock();
                int updatedCount = (*count)--;
                assert(updatedCount > -2);
                if (updatedCount == 0) {
                    delete count;
                    checked_delete();
                }
                mutex->unlock();
                if (updatedCount == 0) {
                    delete mutex;
                }
            }

            void checked_delete() {
                typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
                (void) sizeof(type_must_be_complete);
                delete ptr;
            }

            mutable T *ptr;
            mutable int *count;
            mutable Mutex *mutex;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_SHARED_H_

