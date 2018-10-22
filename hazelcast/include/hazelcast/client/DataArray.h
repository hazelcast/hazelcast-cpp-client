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
#ifndef HAZELCAST_CLIENT_DATAARRAY_H_
#define HAZELCAST_CLIENT_DATAARRAY_H_

#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        template<typename T>
        class DataArray {
        public:
            virtual ~DataArray() { }

            /**
             * @return Returns the number of data items
             */
            virtual size_t size() const = 0;

            /**
             * Please note that this operation MAY (if the de-serialization is not done before) be costly due to
             * de-serialization. It will cache the de-serialized data.
             *
             * @param index The index of the desired item in the array.
             * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
             * @throws IllegalArgumentException If provided index is greater than the maximum array index.
             */
            virtual const T *get(size_t index) = 0;

            /**
             * Will release the de-serialized data.
             *
             * @param index The index of the desired item in the array.
             * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
             * @throws IllegalArgumentException If provided index is greater than the maximum array index.
             */
            virtual std::auto_ptr<T> release(size_t index) = 0;

            /**
             * Please note that this operation MAY (if the de-serialization is not done before) be costly due to
             * de-serialization. It will cache the de-serialized data.
             *
             * @param index The index of the desired item in the array.
             * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
             * @throws IllegalArgumentException If provided index is greater than the maximum array index.
             */
            virtual const T* operator[](size_t index) = 0;
        };
    }
}

#endif //HAZELCAST_CLIENT_DATAARRAY_H_

