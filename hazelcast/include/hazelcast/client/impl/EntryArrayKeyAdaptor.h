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
//
// Created by ihsan demir on 25 02, 2016.
//
#ifndef HAZELCAST_CLIENT_ADAPTOR_IMPL_ENTRYARRAYKEYADAPTOR_H_
#define HAZELCAST_CLIENT_ADAPTOR_IMPL_ENTRYARRAYKEYADAPTOR_H_

#include <vector>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/DataArray.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            namespace impl {
                template<typename K, typename V>
                class EntryArrayKeyAdaptor : public DataArray<K> {
                public:
                    EntryArrayKeyAdaptor(std::unique_ptr<EntryArray<K, V> > &array) : entryArray(std::move(array)) {
                    }


                    virtual ~EntryArrayKeyAdaptor() {
                    }

                    /**
                     * @return Returns the number of data items
                     */
                    size_t size() const {
                        return entryArray->size();
                    }

                    /**
                     * Please note that this operation MAY (if the de-serialization is not done before) be costly due to
                     * de-serialization. It will cache the de-serialized data.
                     *
                     * @param index The index of the desired item in the array.
                     * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                     * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                     */
                    const K *get(size_t index) {
                        return entryArray->getKey(index);
                    }

                    /**
                     * Will release the de-serialized data.
                     *
                     * @param index The index of the desired item in the array.
                     * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                     * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                     */
                    std::unique_ptr<K> release(size_t index) {
                        return entryArray->releaseKey(index);
                    }

                    /**
                     * Please note that this operation MAY (if the de-serialization is not done before) be costly due to
                     * de-serialization. It will cache the de-serialized data.
                     *
                     * @param index The index of the desired item in the array.
                     * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                     * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                     */
                    const K* operator[](size_t index) {
                        return entryArray->getKey(index);
                    }

                private:
                    std::unique_ptr<EntryArray<K, V> > entryArray;

                    // prevent copy operations
                    EntryArrayKeyAdaptor(const EntryArrayKeyAdaptor<K, V> &rhs);

                    EntryArrayKeyAdaptor &operator=(const EntryArrayKeyAdaptor<K, V> &rhs);
                };
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_ADAPTOR_IMPL_ENTRYARRAYKEYADAPTOR_H_

