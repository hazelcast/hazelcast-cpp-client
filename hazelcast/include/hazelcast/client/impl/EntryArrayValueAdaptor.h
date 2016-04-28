/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_ADAPTOR_IMPL_ENTRYARRAYVALUEADAPTOR_H_
#define HAZELCAST_CLIENT_ADAPTOR_IMPL_ENTRYARRAYVALUEADAPTOR_H_

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
                class EntryArrayValueAdaptor : public DataArray<V> {
                public:
                    EntryArrayValueAdaptor(std::auto_ptr<EntryArray<K, V> > array) : entryArray(array) {
                    }


                    virtual ~EntryArrayValueAdaptor() {
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
                    const V *get(size_t index) {
                        return entryArray->getValue(index);
                    }

                    /**
                     * Will release the de-serialized data.
                     *
                     * @param index The index of the desired item in the array.
                     * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                     * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                     */
                    std::auto_ptr<V> release(size_t index) {
                        return entryArray->releaseValue(index);
                    }

                    /**
                     * Please note that this operation MAY (if the de-serialization is not done before) be costly due to
                     * de-serialization. It will cache the de-serialized data.
                     *
                     * @param index The index of the desired item in the array.
                     * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                     * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                     */
                    const V* operator[](size_t index) {
                        return entryArray->getValue(index);
                    }

                private:
                    std::auto_ptr<EntryArray<K, V> > entryArray;

                    // prevent copy operations
                    EntryArrayValueAdaptor(const EntryArrayValueAdaptor<K, V> &rhs);

                    EntryArrayValueAdaptor &operator=(const EntryArrayValueAdaptor<K, V> &rhs);
                };
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_ADAPTOR_IMPL_ENTRYARRAYVALUEADAPTOR_H_

