//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "CountDownLatch.h"


namespace hazelcast {
    namespace util {
        CountDownLatch::CountDownLatch(int count)
        : count(count) {

        }

        void CountDownLatch::countDown() {
            if (count-- == 1) {
                conditionVariable.notify_all();
            }
        }

        bool CountDownLatch::await(long timeInMillis) {
            boost::unique_lock<boost::mutex> lock(mutex);
            if (count == 0) {
                return true;
            }
            boost::cv_status status = conditionVariable.wait_for(lock, boost::chrono::milliseconds(timeInMillis));
            if (status == boost::cv_status::timeout) {
                return false;
            }
            return true;
        }

        void CountDownLatch::await() {
            boost::unique_lock<boost::mutex> lock(mutex);
            if (count == 0) {
                return;
            }
            conditionVariable.wait(lock);
        }
    }
}
