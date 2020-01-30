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

#ifndef HAZELCAST_UTIL_ATOMICARRAY_H_
#define HAZELCAST_UTIL_ATOMICARRAY_H_

#include <vector>
#include <memory>

#include "hazelcast/client/exception/ProtocolExceptions.h"

namespace hazelcast {
    namespace util {
        template<typename T>
        class AtomicArray {
        public:
            /**
             * Creates a new AtomicArray of the given length. The array is kept on a continous memory block.
             *
             * @param length the length of the array
             */
            AtomicArray(size_t length) : array(length) {
                locks = new std::mutex[length];
                for (size_t i = 0; i < length; ++i) {
                    array[i] = 0;
                }
            }

            virtual ~AtomicArray() {
                delete[] locks;
            }

            /**
             * Gets the current value at position {@code i}.
             *
             * @param i the index
             * @return the current value
             */
            T get(size_t i) {
                checkIndexBound(i);
                std::lock_guard guard(locks[i]);
                return array[i];
            }

            /**
             * Atomically adds the given value to the element at index {@code i}.
             *
             * @param i the index
             * @param delta the value to add
             * @return the previous value
             */
            T getAndAdd(size_t i, T delta) {
                checkIndexBound(i);
                std::lock_guard guard(locks[i]);
                T value = array[i];
                array[i] = value + delta;
                return value;
            }

            /**
             * Atomically increments by one the element at index {@code i}.
             *
             * @param i the index
             * @return the updated value
             */
            T incrementAndGet(size_t i) {
                return getAndAdd(i, 1) + 1;
            }

        private:
            // prevent copy construction
            AtomicArray(const AtomicArray &rhs);
            void operator = (const AtomicArray &rhs);

            std::vector<T> array;
            std::mutex *locks;

            void checkIndexBound(size_t i) {
                if (i >= array.size()) {
                    throw (client::exception::ExceptionBuilder<client::exception::IndexOutOfBoundsException>(
                            "AtomicArray::checkIndexBound") << "index " << i).build();
                }
            }
        };
    }
}

#endif //HAZELCAST_UTIL_ATOMICLONGARRAY_H_

