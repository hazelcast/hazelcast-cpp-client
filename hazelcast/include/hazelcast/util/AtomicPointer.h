//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ATOMIC_REFERENCE
#define HAZELCAST_ATOMIC_REFERENCE

#include <iosfwd>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include "LockSupport.h"

namespace hazelcast {
    namespace util {

        template <typename T>
        class AtomicPointer {
        public:
            AtomicPointer()
            : accessLock(LockSupport::getLock(0)) {
            };

            AtomicPointer(T *const p)
            :pointer(p)
            , accessLock(LockSupport::getLock((*pointer).get()->hashCode())) {

            };

            AtomicPointer(const AtomicPointer &rhs)
            :pointer(rhs.pointer)
            , accessLock(rhs.accessLock) {
            };

            void operator = (const AtomicPointer &rhs) {
                (*pointer) = *(rhs.pointer);
                accessLock = rhs.accessLock;
            };

            ~AtomicPointer() {
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                (*pointer).reset();
            };

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
