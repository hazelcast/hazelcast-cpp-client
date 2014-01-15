//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_QUEUE
#define HAZELCAST_CONCURRENT_QUEUE

#include "hazelcast/util/HazelcastDll.h"
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>
#include <queue>
#include <iostream>

namespace hazelcast {
    namespace util {
        template <typename T>
        /* Non blocking - synchronized queue - does not delete memory except at destructor  */
        class HAZELCAST_API ConcurrentQueue {
        public:
            ConcurrentQueue() {

            };

            ~ConcurrentQueue() {
                T *ptr;
                while ((ptr = poll()) != NULL) {
                    delete ptr;
                }
            };


            void offer(T *e) {
                boost::lock_guard<boost::mutex> lg(m);
                internalQueue.push(e);
            };

            T *poll() {
                T *e = NULL;
                boost::lock_guard<boost::mutex> lg(m);
                if (!internalQueue.empty()) {
                    e = internalQueue.front();
                    internalQueue.pop();
                }
                return e;
            };

        private:
            boost::mutex m;
            std::queue<T *> internalQueue;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_QUEUE
