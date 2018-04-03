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
#ifndef HAZELCAST_ATOMIC_NUMBER
#define HAZELCAST_ATOMIC_NUMBER

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include <string>
#include <stdint.h>

namespace hazelcast {
    namespace client {
        namespace spi {
            class ClientContext;
        }

        /**
        * IAtomicLong is a distributed atomic long implementation.
        * Note that, in node failures atomic long will be restored
        * via backup.
        *
        */
        class HAZELCAST_API IAtomicLong : public proxy::ProxyImpl {
            friend class HazelcastClient;

            friend class IdGenerator;

        public:

            /**
            * adds the given value to the current value.
            *
            * @param delta the value to add
            * @return the updated value
            */
            int64_t addAndGet(int64_t delta);

            /**
            * sets the value to the given updated value
            * only if the current value is equal to the expected value.
            *
            * @param expect the expected value
            * @param update the new value
            * @return true if successful; or false if the actual value
            *         was not equal to the expected value.
            */
            bool compareAndSet(int64_t expect, int64_t update);

            /**
            * decrements the current value by one.
            *
            * @return the updated value
            */
            int64_t decrementAndGet();

            /**
            * Gets the current value.
            *
            * @return the current value
            */
            int64_t get();

            /**
            * adds the given value to the current value.
            *
            * @param delta the value to add
            * @return the old value before the add
            */
            int64_t getAndAdd(int64_t delta);

            /**
            * sets the given value and returns the old value.
            *
            * @param newValue the new value
            * @return the old value
            */
            int64_t getAndSet(int64_t newValue);

            /**
            * increments the current value by one.
            *
            * @return the updated value
            */
            int64_t incrementAndGet();

            /**
            * increments the current value by one.
            *
            * @return the old value
            */
            int64_t getAndIncrement();

            /**
            * sets the given value.
            *
            * @param newValue the new value
            */
            void set(int64_t newValue);

        private:

            IAtomicLong(const std::string& objectName, spi::ClientContext *context);

            int partitionId;
        };
    }
}

#endif /* HAZELCAST_ATOMIC_NUMBER */
