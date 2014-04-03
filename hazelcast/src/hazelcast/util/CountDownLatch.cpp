//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/util/Util.h"
#include "hazelcast/util/CountDownLatch.h"
#include <unistd.h>

namespace hazelcast {
    namespace util {
        CountDownLatch::CountDownLatch(int count)
        : count(count) {

        }

        void CountDownLatch::countDown() {
            count--;
        }

        bool CountDownLatch::await(long timeInMillis) {
            long endTime = util::getCurrentTimeMillis() + timeInMillis;
            while (endTime > util::getCurrentTimeMillis()) {
                if (count == 0) {
                    return true;
                }
                ::sleep(1);
            }
            if (count == 0) {
                return true;
            }
            return false;
        }

        void CountDownLatch::await() {
            while (true) {
                if (count == 0) {
                    break;
                }
                ::sleep(1);
            };
        }
    }
}
