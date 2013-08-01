//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ATOMIC_REFERENCE
#define HAZELCAST_ATOMIC_REFERENCE

#include <iosfwd>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/recursive_mutex.hpp>

namespace hazelcast {
    namespace util {

        template <typename T>
        class AtomicPointer {
        public:
            AtomicPointer()
            : accessLock(new boost::recursive_mutex) {
            };

            AtomicPointer(T *const p)
            :pointer(p)
            , accessLock(new boost::recursive_mutex) {

            };

            AtomicPointer(const AtomicPointer &rhs) {
                boost::lock_guard<boost::recursive_mutex> lg(*(rhs.accessLock));
                pointer = rhs.pointer;
                accessLock = rhs.accessLock;
            };

            void operator = (const AtomicPointer &rhs) {
                boost::lock_guard<boost::recursive_mutex> lg(*(rhs.accessLock));
                pointer = rhs.pointer;
                accessLock = rhs.accessLock;
            };

            ~AtomicPointer() {

            };

            bool operator ==(const AtomicPointer &rhs) const{
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                return pointer == rhs.pointer;
            };

            bool operator !=(const AtomicPointer &rhs) const{
                return !(pointer == rhs.pointer);
            };

            void lock() {
                accessLock->lock();
            };

            void unlock() {
                accessLock->unlock();
            };

            T operator *() {
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                return *pointer;
            };

            T *operator ->() const{
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                return pointer.get();
            };

            T *get() {
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                return pointer.get();
            };

            void reset() {
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                pointer.reset();
            };

            void reset(T *p) {
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                pointer.reset(p);
            };

            int use_count() { //for debug purposes
                boost::lock_guard<boost::recursive_mutex> lg(*accessLock);
                return pointer.use_count();
            }

        private:
            mutable boost::shared_ptr<T> pointer;
            mutable boost::shared_ptr<boost::recursive_mutex> accessLock;
        };

    }
}


#endif //HAZELCAST_ATOMIC_REFERENCE
