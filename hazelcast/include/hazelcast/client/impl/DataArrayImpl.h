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
#ifndef HAZELCAST_CLIENT_IMPL_DATAARRAYIMPL_H_
#define HAZELCAST_CLIENT_IMPL_DATAARRAYIMPL_H_

#include <vector>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/DataArray.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            template<typename T>
            class DataArrayImpl : public DataArray<T> {
            public:
                DataArrayImpl(const std::vector<serialization::pimpl::Data> &data,
                              serialization::pimpl::SerializationService &service) : values(data),
                                                                                     serializationService(service){
                    for (std::vector<serialization::pimpl::Data>::const_iterator it = values.begin();it != values.end(); ++it) {
                        struct Item item;
                        item.data = &(*it);
                        item.isDeserialized = false;
                        item.value = NULL;
                        item.serializationService = &serializationService;
                        deserializedEntries.push_back(item);
                    }
                }

                virtual ~DataArrayImpl() {
                }

                /**
                 * @return Returns the number of data items
                 */
                size_t size() const {
                    return deserializedEntries.size();
                }

                /**
                 * Please note that this operation MAY (if the de-serialization is not done before) be costly due to
                 * de-serialization. It will cache the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                const T *get(size_t index) {
                    checkIndex(index);
                    return deserializedEntries[index].get();
                }

                /**
                 * Will release the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                std::unique_ptr<T> release(size_t index) {
                    checkIndex(index);
                    return deserializedEntries[index].release();
                }

                /**
                 * Please note that this operation MAY (if the de-serialization is not done before) be costly due to
                 * de-serialization. It will cache the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the pointer for the newly created object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                const T* operator[](size_t index) {
                    return get(index);
                }

            private:
                struct Item {
                    const serialization::pimpl::Data *data;
                    bool isDeserialized;
                    T *value;
                    serialization::pimpl::SerializationService *serializationService;

                    ~Item() {
                        if (isDeserialized) {
                            delete value;
                        }
                    }

                    T *get() {
                        if (isDeserialized) {
                            return value;
                        }

                        value = serializationService->toObject<T>(*data).release();
                        isDeserialized = true;
                        return value;
                    }

                    std::unique_ptr<T> release() {
                        if (isDeserialized) {
                            std::unique_ptr<T> result(value);
                            isDeserialized = false;
                            value = NULL;
                            return result;
                        }

                        return serializationService->toObject<T>(*data);
                    }
                };

                std::vector<serialization::pimpl::Data> values;
                serialization::pimpl::SerializationService &serializationService;
                std::vector<struct Item> deserializedEntries;

                /**
                 *  @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                void checkIndex(size_t index) const {
                    size_t len = values.size();
                    if (0 == len) {
                        char msg[200];
                        util::hz_snprintf(msg, 200,
                                          "The are no elements in the array, you should not try accessing any element of the "
                                                  "array. Provided index (%lu) id out of range.", index);
                        throw client::exception::IllegalArgumentException("DataArrayImpl", msg);
                    }

                    if (index >= len) {
                        char msg[200];
                        util::hz_snprintf(msg, 200,
                                          "Provided index (%lu) id out of range. Maximum allowed index is %lu",
                                          index, (len - 1));
                        throw client::exception::IllegalArgumentException("DataArrayImpl", msg);
                    }
                }

                // prevent copy operations
                DataArrayImpl(const DataArrayImpl<T> &rhs);

                DataArrayImpl &operator=(const DataArrayImpl<T> &rhs);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_DATAARRAYIMPL_H_

