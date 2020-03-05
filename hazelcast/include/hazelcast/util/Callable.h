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

#ifndef HAZELCAST_UTIL_CALLABLE_H_
#define HAZELCAST_UTIL_CALLABLE_H_

#include <memory>

#include "hazelcast/util/Named.h"

namespace hazelcast {
    namespace util {
        /**
         * A task that returns a result and may throw an exception.
         * Implementors define a single method with no arguments called
         * {@code call}.
         *
         * <p>The {@code Callable} interface is similar to {@link
         * Runnable}, in that both are designed for classes whose
         * instances are potentially executed by another thread.  A
         * {@code Runnable}, however, does not return a result and cannot
         * throw a checked exception.
         *
         * @param <V> the result type of method {@code call}
         */
        template <typename V>
        class Callable : public Named {
        public:
            virtual std::shared_ptr<V> call() = 0;
        };
    }
}


#endif //HAZELCAST_UTIL_CALLABLE_H_
