//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_QUEUE
#define HAZELCAST_CONCURRENT_QUEUE

#include <boost/lockfree/queue.hpp>

namespace hazelcast {
    namespace client {
        namespace util {
            template <typename T>
            class ConcurrentQueue {
            public:
                ConcurrentQueue() {

                };

                bool offer(const T& e) {
                    return queue.push(e);;
                };

                bool poll(T& e) {
                    return queue.pop(e);
                };

                bool empty() {
                    return queue.empty();
                };

            private:
                boost::lockfree::queue<T> queue;
            };
        }
    }
}

#endif //HAZELCAST_CONCURRENT_QUEUE
