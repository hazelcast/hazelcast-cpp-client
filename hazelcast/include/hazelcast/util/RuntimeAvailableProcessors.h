/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_UTIL_RUNTIMEAVAILABLEPROCESSORS_H_
#define HAZELCAST_UTIL_RUNTIMEAVAILABLEPROCESSORS_H_

#include "hazelcast/util/AtomicInt.h"

namespace hazelcast {
    namespace util {
        /**
         * Utility class to access available processors in the system and optionally override its return value.
         * <p>
         * This class makes the number of available processors configurable for the sake of testing.
         * {@link #override(int)} and {@link #resetOverride()} should only be used for testing purposes.
         */

        class HAZELCAST_API RuntimeAvailableProcessors {
        public:
            /**
             * Returns the number of available processors.
             * <p>
             * Returned value is either equal to {@link getNumberOfProcessors()}
             * or an overridden value by call of the method {@link #override(int)}.
             *
             * @return number of available processors
             */
            static int get();

            /**
             * Overrides the number of available processors returned by the method {@link #get()}.
             * <p>
             * This is to be used only for testing.
             *
             * @param availableProcessors number of available processors
             */
            static void override(int availableProcessors);

            /**
             * This is to be used only for testing.
             *
             * @return The number of available processors on the platform.
             */
            static int getNumberOfProcessors();

            /**
             * Resets the overridden number of available processors to {@link getNumberOfProcessors()}.
             * <p>
             * This is to be used only for testing.
             */
            static void resetOverride();
        private:

            // number of available processors currently configured
            static util::AtomicInt currentAvailableProcessors;

        };
    }
}
#endif //HAZELCAST_UTIL_RUNTIMEAVAILABLEPROCESSORS_H_
