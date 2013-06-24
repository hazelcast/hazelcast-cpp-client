//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_QUEUE
#define HAZELCAST_CONCURRENT_QUEUE

#include "Lock.h"
#include "LockGuard.h"
#include <queue>
#include <iostream>

namespace hazelcast {
    namespace util {
        template <typename T>
        class ConcurrentQueue {
        public:
            ConcurrentQueue() {

            };

            bool offer(const T& e) {
                util::LockGuard lg(mutex);
                internalQueue.push(e);
                return true;
            };

            bool poll(T& e) {
                util::LockGuard lg(mutex);
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
            util::Lock mutex;
            std::queue<T> internalQueue;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_QUEUE
