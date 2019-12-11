/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_UTIL_CONCURRENT_CANCELLABLE_H_
#define HAZELCAST_UTIL_CONCURRENT_CANCELLABLE_H_

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        namespace concurrent {
            class HAZELCAST_API Cancellable {
            public:
                /**
                 * Attempts to cancel execution of this task.  This attempt will
                 * fail if the task has already completed, has already been cancelled,
                 * or could not be cancelled for some other reason.
                 *
                 * <p>After this method returns, subsequent calls to {@link #isCancelled}
                 * will always return {@code true} if this method returned {@code true}.
                 *
                 *
                 * @return {@code false} if the task could not be cancelled,
                 * typically because it has already completed normally;
                 * {@code true} otherwise
                 */
                virtual bool cancel() = 0;

                /**
                 * Returns {@code true} if this task was cancelled before it completed
                 * normally.
                 *
                 * @return {@code true} if this task was cancelled before it completed
                 */
                virtual bool isCancelled() = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_CONCURRENT_CANCELLABLE_H_
