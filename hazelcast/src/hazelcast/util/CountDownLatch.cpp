//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/util/CountDownLatch.h"


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
            util::LockGuard lock(mutex);
            if (count == 0) {
                return true;
            }
            util::ConditionVariable::status status = conditionVariable.wait_for(mutex, timeInMillis);
            if (status == util::ConditionVariable::timeout) {
                return false;
            }
            return true;
        }

        void CountDownLatch::await() {
            util::LockGuard lock(mutex);
            if (count == 0) {
                return;
            }
            conditionVariable.wait(mutex);
        }
    }
}
