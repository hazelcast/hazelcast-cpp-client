//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.

#ifndef HAZELCAST_CONCURRENT_QUEUE
#define HAZELCAST_CONCURRENT_QUEUE

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/LockGuard.h"
#include "hazelcast/util/Mutex.h"
#include <queue>
#include <iostream>

namespace hazelcast {
    namespace util {
        template <typename T>
        /* Non blocking - synchronized queue - does not delete memory ever */
        class HAZELCAST_API ConcurrentQueue {
        public:
            ConcurrentQueue() {

            }

            void offer(T *e) {
                util::LockGuard lg(m);
                internalQueue.push(e);
            }

            T *poll() {
                T *e = NULL;
                util::LockGuard lg(m);
                if (!internalQueue.empty()) {
                    e = internalQueue.front();
                    internalQueue.pop();
                }
                return e;
            }

        private:
            util::Mutex m;
            std::queue<T *> internalQueue;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_QUEUE

