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

#ifndef HAZELCAST_CLIENT_IMPL_ATOMICLONGINTERFACE_H_
#define HAZELCAST_CLIENT_IMPL_ATOMICLONGINTERFACE_H_

#include <stdint.h>

#include "hazelcast/client/DistributedObject.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API AtomicLongInterface : public virtual DistributedObject {
            public:
                /**
                * adds the given value to the current value.
                *
                * @param delta the value to add
                * @return the updated value
                */
                virtual int64_t addAndGet(int64_t delta) = 0;

                /**
                * sets the value to the given updated value
                * only if the current value is equal to the expected value.
                *
                * @param expect the expected value
                * @param update the new value
                * @return true if successful; or false if the actual value
                *         was not equal to the expected value.
                */
                virtual bool compareAndSet(int64_t expect, int64_t update) = 0;

                /**
                * decrements the current value by one.
                *
                * @return the updated value
                */
                virtual int64_t decrementAndGet() = 0;

                /**
                * Gets the current value.
                *
                * @return the current value
                */
                virtual int64_t get() = 0;

                /**
                * adds the given value to the current value.
                *
                * @param delta the value to add
                * @return the old value before the add
                */
                virtual int64_t getAndAdd(int64_t delta) = 0;

                /**
                * sets the given value and returns the old value.
                *
                * @param newValue the new value
                * @return the old value
                */
                virtual int64_t getAndSet(int64_t newValue) = 0;

                /**
                * increments the current value by one.
                *
                * @return the updated value
                */
                virtual int64_t incrementAndGet() = 0;

                /**
                * increments the current value by one.
                *
                * @return the old value
                */
                virtual int64_t getAndIncrement() = 0;

                /**
                * sets the given value.
                *
                * @param newValue the new value
                */
                virtual void set(int64_t newValue) = 0;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_ATOMICLONGINTERFACE_H_

