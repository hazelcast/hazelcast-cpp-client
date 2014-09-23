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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_CONCURRENT_QUEUE

