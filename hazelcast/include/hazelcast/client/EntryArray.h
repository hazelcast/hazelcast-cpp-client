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
//
// Created by ihsan demir on 25 02, 2016.
//
#ifndef HAZELCAST_CLIENT_ENTRYARRAY_H_
#define HAZELCAST_CLIENT_ENTRYARRAY_H_

#include <vector>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "DataArray.h"

namespace hazelcast {
    namespace client {
        template<typename K, typename V>
        class EntryArray {
        public:
            virtual ~EntryArray() { }

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
            virtual std::auto_ptr<K> releaseKey(size_t index) = 0;

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
            virtual std::auto_ptr<V> releaseValue(size_t index) = 0;

            /**
             * @param index The index of the desired item in the array.
             * @return The key value pointer pair is returned
             */
            virtual std::pair<const K *, const V *> operator[](size_t index) = 0;

            /**
             * Sorts the entries using the comparator if comparator is not null. Otherwise, sorts based on the provided
             * iterationType value.
             * @param iterationType if null comparator is provided then iteration type is query::VALUE, the entry value
             * should not be null and the entries shall be sorted by value, otherwise the entries shall be sorted by key.
             */
            virtual void sort(query::IterationType iterationType, const util::Comparator<std::pair<const K *, const V *> > *comparator = 0) = 0;
        };
    }
}

#endif //HAZELCAST_CLIENT_ENTRYARRAY_H_

