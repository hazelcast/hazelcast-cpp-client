//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_QUEUE
#define HAZELCAST_CONCURRENT_QUEUE

#include <queue>
#include <iostream>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/mutex.hpp>

namespace hazelcast {
    namespace util {
        template <typename T>
        class ConcurrentQueue {
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
                if (!empty()) {
                    e = internalQueue.front();
                    internalQueue.pop();
                } else {
                    success = false;
                }
                return success;
            };

            bool empty() {
                return internalQueue.empty();
            };

        private:
            boost::mutex m;
            std::queue<T> internalQueue;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_QUEUE
