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
#ifndef HAZELCAST_UTIL_CONCURRENT_LOCKS_LOCKSUPPORT_H_
#define HAZELCAST_UTIL_CONCURRENT_LOCKS_LOCKSUPPORT_H_

#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        namespace concurrent {
            namespace locks {
                 class LockSupport {
                public:
                     /**
                      * Disables the current thread for thread scheduling purposes, for up to
                      * the specified waiting time, unless the permit is available.
                      *
                      * <p>If the permit is available then it is consumed and the call
                      * returns immediately; otherwise the current thread becomes disabled
                      * for thread scheduling purposes and lies dormant until one of four
                      * things happens:
                      *
                      * <ul>
                      * <li>Some other thread invokes {@link #unpark unpark} with the
                      * current thread as the target; or
                      *
                      * <li>Some other thread {@linkplain Thread#interrupt interrupts}
                      * the current thread; or
                      *
                      * <li>The specified waiting time elapses; or
                      *
                      * <li>The call spuriously (that is, for no reason) returns.
                      * </ul>
                      *
                      * <p>This method does <em>not</em> report which of these caused the
                      * method to return. Callers should re-check the conditions which caused
                      * the thread to park in the first place. Callers may also determine,
                      * for example, the interrupt status of the thread, or the elapsed time
                      * upon return.
                      *
                      * @param nanos the maximum number of nanoseconds to wait
                      */
                    static void parkNanos(int64_t nanos);
                };
            }
        }
    }
}

#endif //HAZELCAST_UTIL_CONCURRENT_LOCKS_LOCKSUPPORT_H_
