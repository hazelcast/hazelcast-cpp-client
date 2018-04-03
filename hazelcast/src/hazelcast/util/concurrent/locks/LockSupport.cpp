/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/ConditionVariable.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"

namespace hazelcast {
    namespace util {
        namespace concurrent {
            namespace locks {

                void LockSupport::parkNanos(int64_t nanos) {
                    if (nanos <= 0) {
                        return;
                    }

                    Mutex lock;
                    ConditionVariable conditionVariable;
                    conditionVariable.waitNanos(lock, nanos);
                }
            }
        }
    }
}
