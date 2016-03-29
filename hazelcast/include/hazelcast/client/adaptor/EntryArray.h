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
#ifndef HAZELCAST_CLIENT_ADAPTOR_ENTRYARRAY_H_
#define HAZELCAST_CLIENT_ADAPTOR_ENTRYARRAY_H_

#include <vector>

#include "hazelcast/util/Util.h"
#include <hazelcast/client/exception/IllegalArgumentException.h>
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace adaptor {
            template <typename K, typename V>
            class EntryArray {
            public:
                EntryArray(const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries,
                        serialization::pimpl::SerializationService &service) : dataEntries(entries),
                                                                               serializationService(service) {
                }

                /**
                 * @return Returns the number of data items
                 */
                size_t size() const {
                    return dataEntries.size();
                }

                /**
                 * Please note that this operation is costly due to de-serialization. It will NOT cache the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the key object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                std::auto_ptr<K> getKey(size_t index) const {
                    checkIndex(index);
                    return serializationService.toObject<K>(dataEntries[index].first);
                }

                /**
                 * Please note that this operation is costly due to de-serialization. It will NOT cache the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the value object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                std::auto_ptr<V> getValue(size_t index) const {
                    checkIndex(index);
                    return serializationService.toObject<V>(dataEntries[index].second);
                }
            private:
                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataEntries;
                serialization::pimpl::SerializationService &serializationService;

                /**
                 *  @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                void checkIndex(size_t index) const {
                    size_t len = dataEntries.size();
                    if (0 == len) {
                        char msg[200];
                        util::snprintf(msg, 200,
                                       "The are no elements in the array, you should not try accessing any element of the "
                                               "array. Provided index (%lu) id out of range.", index);
                        throw client::exception::IllegalArgumentException("DataArray", msg);
                    }

                    if (index >= len) {
                        char msg[200];
                        util::snprintf(msg, 200, "Provided index (%lu) id out of range. Maximum allowed index is %lu",
                                       index, (len - 1));
                        throw client::exception::IllegalArgumentException("EntryArray", msg);
                    }

                }

                // prevent copy operations
                EntryArray(const EntryArray &rhs);
                EntryArray &operator =(const EntryArray &rhs);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_ADAPTOR_ENTRYARRAY_H_

