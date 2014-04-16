//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/util/Util.h"
#include "hazelcast/util/CountDownLatch.h"
#include <ctime> 
namespace hazelcast {
    namespace util {
        CountDownLatch::CountDownLatch(int count)
        : count(count) {

        }

        void CountDownLatch::countDown() {
            count--;
        }

        bool CountDownLatch::await(int seconds) {
            time_t endTime = time(NULL) + seconds;
            while (endTime > time(NULL)) {
                if (count == 0) {
                    return true;
                }
                sleep(1);
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
                util::sleep(1);
            };
        }
    }
}

