//
// Created by sancar koyunlu on 5/29/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ATOMIC_REFERENCE
#define HAZELCAST_ATOMIC_REFERENCE

#include <iosfwd>
#include <boost/shared_ptr.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

namespace hazelcast {
    namespace util {
        template <typename T>
        class AtomicPointer {
        public:
            AtomicPointer():pointer(NULL) {
            };

            AtomicPointer(T *p):pointer(p) {
            };

            const boost::shared_ptr<T> get() {
                boost::lock_guard<boost::mutex> lg(mutex);
                return pointer;
            };

            void set(T *p) {
                boost::lock_guard<boost::mutex> lg(mutex);
                boost::shared_ptr<T> newPointer(p);
                pointer = newPointer;
            };

        private:
            boost::shared_ptr<T> pointer;
            boost::mutex mutex;
        };

    }
}


#endif //HAZELCAST_ATOMIC_REFERENCE
