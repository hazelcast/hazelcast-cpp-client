//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ATOMIC_REFERENCE
#define HAZELCAST_ATOMIC_REFERENCE

#include "hazelcast/util/LockSupport.h"
#include <iosfwd>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace hazelcast {
    namespace util {

        template <typename T>
        class HAZELCAST_API AtomicPointer {
        public:
            AtomicPointer()
            : pointer(new boost::shared_ptr<T>())
            , accessLock(LockSupport::getLock(rand())) {//TODO ?? Rand()
            };

            AtomicPointer(T *const p, int lockId)
            : pointer(new boost::shared_ptr<T>(p))
            , accessLock(LockSupport::getLock(lockId)) {
            };

            AtomicPointer(const AtomicPointer &rhs)
            :pointer(new boost::shared_ptr<T>()) {
                *this = rhs;
            };

            void operator = (const AtomicPointer &rhs) {
                boost::lock_guard<boost::recursive_mutex> lg(*(rhs.accessLock));
                (*pointer) = *(rhs.pointer);
                accessLock = rhs.accessLock;
            };

            ~AtomicPointer() {
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                (*pointer).reset();
                delete pointer;
            };

            bool isNull() const {
                T *ptr = pointer->get();
                return ptr == NULL;
            }

            bool operator ==(const AtomicPointer &rhs) const {
                return (*pointer) == *(rhs.pointer);
            };

            bool operator !=(const AtomicPointer &rhs) const {
                return !(*this == rhs);
            };

            void lock() {
                accessLock->lock();
            };

            void unlock() {
                accessLock->unlock();
            };

            T& operator *() {
                return *(*pointer);
            };

            T *operator ->() const {
                return (*pointer).get();
            };

            T *get() {
                return (*pointer).get();
            };

            int use_count() { //for debug purposes
                return (*pointer).use_count();
            }

        private:
            boost::shared_ptr<T> *const pointer;
            boost::recursive_mutex *accessLock;
        };

    }
}


#endif //HAZELCAST_ATOMIC_REFERENCE
