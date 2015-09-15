//
//  Created by ihsan demir on 9/9/15.
//  Copyright (c) 2015 ihsan demir. All rights reserved.
//

#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace util {
        LockGuard::LockGuard(Mutex &mutex) : mutex(mutex) {
                mutex.lock();
        }

        LockGuard::~LockGuard() {
            mutex.unlock();
        }
    }
}

