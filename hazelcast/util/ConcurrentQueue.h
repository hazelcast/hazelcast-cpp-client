//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_QUEUE
#define HAZELCAST_CONCURRENT_QUEUE

#include <queue>

namespace hazelcast {
    namespace util {
        template <typename T>
        class ConcurrentQueue {
        public:
            ConcurrentQueue() {

            };

            bool offer(const T& e) {
                queue.push(e);
                return true;
            };

            bool poll(T& e) {
                if (!empty()) {
                    e = queue.front();
                    queue.pop();
                    return true;
                } else
                    return false;
            };

            bool empty() {
                return queue.empty();
            };

        private:
            std::queue<T> queue;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_QUEUE
