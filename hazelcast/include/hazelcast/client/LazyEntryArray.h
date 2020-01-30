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
#ifndef HAZELCAST_CLIENT_LAZYENTRYARRAY_H_
#define HAZELCAST_CLIENT_LAZYENTRYARRAY_H_

#include <memory>

namespace hazelcast {
    namespace client {
        template<typename K, typename V>
        class LazyEntryArray {
        public:
            virtual ~LazyEntryArray() {}

            /**
             * @return Returns the number of data items
             */
            virtual size_t size() const = 0;

            /**
             * Please note that this operation is costly due to de-serialization. It caches deserialized data.
             *
             * @param index The index of the desired item in the array.
             * @return Deserializes the data and returns the key object for the data at the provided index.
             * @throws IllegalArgumentException If provided index is greater than the maximum array index.
             */
            virtual const K *getKey(size_t index) = 0;

            /**
             * Please note that this operation MAY(if not deserialized previously) be costly due to de-serialization.
             * It will NOT cache the de-serialized data.
             *
             * @param index The index of the desired item in the array.
             * @return Deserializes the data and returns the key object for the data at the provided index.
             * @throws IllegalArgumentException If provided index is greater than the maximum array index.
             */
            virtual std::unique_ptr<K> releaseKey(size_t index) = 0;

            /**
             * Please note that this operation is costly due to de-serialization. It will cache the de-serialized data.
             *
             * @param index The index of the desired item in the array.
             * @return Deserializes the data and returns the value object for the data at the provided index.
             * @throws IllegalArgumentException If provided index is greater than the maximum array index.
             */
            virtual const V *getValue(size_t index) = 0;

            /**
             * Please note that this operation is costly due to de-serialization. It will NOT cache the de-serialized data.
             *
             * @param index The index of the desired item in the array.
             * @return Deserializes the data and returns the value object for the data at the provided index.
             * @throws IllegalArgumentException If provided index is greater than the maximum array index.
             */
            virtual std::unique_ptr<V> releaseValue(size_t index) = 0;

            /**
             * @param index The index of the desired item in the array.
             * @return The key value pointer pair is returned
             */
            virtual std::pair<const K *, const V *> operator[](size_t index) = 0;
        };
    }
}

#endif //HAZELCAST_CLIENT_LAZYENTRYARRAY_H_

