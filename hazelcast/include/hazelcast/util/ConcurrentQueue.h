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
        class HAZELCAST_API ConcurrentQueue {
        public:
            ConcurrentQueue() {

            };

            bool offer(const T& e) {
                boost::lock_guard<boost::mutex> lg(m);
                internalQueue.push(e);
                return true;
            };

            bool poll(T& e) {
                boost::lock_guard<boost::mutex> lg(m);
                bool success = true;
                if (!internalQueue.empty()) {
                    e = internalQueue.front();
                    internalQueue.pop();
                } else {
                    success = false;
                }
                return success;
            };

            bool empty() {
                boost::lock_guard<boost::mutex> lg(m);
                return internalQueue.empty();
            };

        private:
            boost::mutex m;
            std::queue<T> internalQueue;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_QUEUE
