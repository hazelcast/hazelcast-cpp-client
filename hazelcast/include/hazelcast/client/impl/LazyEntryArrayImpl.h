/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_IMPL_LAZYENTRYARRAYIMPL_H_
#define HAZELCAST_CLIENT_IMPL_LAZYENTRYARRAYIMPL_H_

#include <vector>

#include "hazelcast/util/Util.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/LazyEntryArray.h"
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            template<typename K, typename V>
            class LazyEntryArrayImpl : virtual public LazyEntryArray<K, V> {
            public:
                LazyEntryArrayImpl(
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries,
                        serialization::pimpl::SerializationService &service) : dataEntries(entries),
                                                                               serializationService(service) {
                    for (std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >::const_iterator it =
                            dataEntries.begin();it != dataEntries.end(); ++it) {
                        struct Item item;
                        item.data = &(*it);
                        item.isValueDeserialized = false;
                        item.isKeyDeserialized = false;
                        item.key = NULL;
                        item.value = NULL;
                        item.serializationService = &serializationService;
                        item.comparator = NULL;
                        deserializedEntries.push_back(item);
                    }
                }

                /**
                 * Modifies the array. Do not use the array after this call but just use the new object
                 */
                LazyEntryArrayImpl(LazyEntryArrayImpl &array, size_t begin, size_t end) : serializationService(array.serializationService) {
                    if (end < begin) {
                        BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("LazyEntryArrayImpl",
                                                                                  "end should be greater than begin!"));
                    }

                    size_t size = array.size();
                    if (end > size) {
                        BOOST_THROW_EXCEPTION(exception::IllegalArgumentException("LazyEntryArrayImpl",
                                                                                  "end should not be greater than array size!"));
                    }

                    // make sure that items are deserialized
                    for (size_t i = 0; i < size; ++i) {
                        array[i];
                    }

                    for (size_t i = begin; i < end; ++i) {
                        Item &item = array.deserializedEntries[i];
                        dataEntries.push_back(*item.data);
                        deserializedEntries.push_back(item);
                        // invalidate the entry at the original array
                        item.isValueDeserialized = false;
                        item.isKeyDeserialized = false;
                    }
                }

                virtual ~LazyEntryArrayImpl() {
                    for (typename std::vector<Item>::const_iterator it = deserializedEntries.begin();it != deserializedEntries.end(); ++it) {
                        if (it->isKeyDeserialized) {
                            delete it->key;
                        }
                        if (it->isValueDeserialized) {
                            delete it->value;
                        }
                    }
                }

                /**
                 * @return Returns the number of data items
                 */
                size_t size() const {
                    return deserializedEntries.size();
                }

                /**
                 * Please note that this operation is costly due to de-serialization. It caches deserialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the key object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                const K *getKey(size_t index) {
                    checkIndex(index);
                    return deserializedEntries[index].getKey();
                }

                /**
                 * Please note that this operation MAY(if not deserialized previously) be costly due to de-serialization.
                 * It will NOT cache the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the key object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                std::unique_ptr<K> releaseKey(size_t index) {
                    checkIndex(index);
                    return deserializedEntries[index].releaseKey();
                }

                /**
                 * Please note that this operation is costly due to de-serialization. It will cache the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the value object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                const V *getValue(size_t index) {
                    checkIndex(index);
                    return deserializedEntries[index].getValue();
                }

                /**
                 * Please note that this operation is costly due to de-serialization. It will NOT cache the de-serialized data.
                 *
                 * @param index The index of the desired item in the array.
                 * @return Deserializes the data and returns the value object for the data at the provided index.
                 * @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                std::unique_ptr<V> releaseValue(size_t index) {
                    checkIndex(index);
                    return deserializedEntries[index].releaseValue();
                }

                std::pair<const K *, const V *> operator[](size_t index) {
                    checkIndex(index);
                    std::pair<const K *, const V *> result;
                    result.first = getKey(index);
                    result.second = getValue(index);
                    return result;
                }

            protected:
                struct Item {
                    const std::pair<serialization::pimpl::Data, serialization::pimpl::Data> *data;
                    bool isKeyDeserialized;
                    bool isValueDeserialized;
                    K *key;
                    V *value;
                    serialization::pimpl::SerializationService *serializationService;
                    const util::Comparator<std::pair<const K *, const V *> > *comparator;
                    query::IterationType type;

                    const K *getKey() {
                        if (isKeyDeserialized) {
                            return key;
                        }

                        key = serializationService->toObject<K>(data->first).release();
                        isKeyDeserialized = true;
                        return key;
                    }

                    std::unique_ptr<K> releaseKey() {
                        if (isKeyDeserialized) {
                            std::unique_ptr<K> result(key);
                            isKeyDeserialized = false;
                            key = NULL;
                            return result;
                        }

                        return serializationService->toObject<K>(data->first);
                    }

                    const V *getValue() {
                        if (isValueDeserialized) {
                            return value;
                        }

                        value = serializationService->toObject<V>(data->second).release();
                        isValueDeserialized = true;
                        return value;
                    }

                    std::unique_ptr<V> releaseValue() {
                        if (isValueDeserialized) {
                            std::unique_ptr<V> result(value);
                            isValueDeserialized = false;
                            value = NULL;
                            return result;
                        }

                        return serializationService->toObject<V>(data->second);
                    }

                    /**
                     * Compare using the comparator if comparator is provided, otherwise use the key for comparison
                     */
                    bool operator<(const Item &rhs) const {
                        std::pair<const K *, const V *> leftVal(key, value);
                        std::pair<const K *, const V *> rightVal(rhs.key, rhs.value);
                        if (NULL != comparator) {
                            int result = comparator->compare(&leftVal, &rightVal);
                            if (0 != result) {
                                // std sort: comparison function object returns ​true if the first argument is less
                                // than (i.e. is ordered before) the second.
                                return result < 0;
                            }

                            return *key < *rhs.key;
                        }

                        switch(type) {
                            case query::VALUE:
                                return *value < *rhs.value;
                            default:
                                return *key < *rhs.key;
                        }
                    }
                };

                /**
                 *  @throws IllegalArgumentException If provided index is greater than the maximum array index.
                 */
                void checkIndex(size_t index) const {
                    size_t len = dataEntries.size();
                    if (0 == len) {
                        char msg[200];
                        util::hz_snprintf(msg, 200,
                                          "The are no elements in the array, you should not try accessing any element of the "
                                                  "array. Provided index (%lu) id out of range.", index);
                        throw client::exception::IllegalArgumentException("DataArray", msg);
                    }

                    if (index >= len) {
                        char msg[200];
                        util::hz_snprintf(msg, 200,
                                          "Provided index (%lu) id out of range. Maximum allowed index is %lu",
                                          index, (len - 1));
                        throw client::exception::IllegalArgumentException("LazyEntryArrayImpl::checkIndex", msg);
                    }

                }

                std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > dataEntries;
                serialization::pimpl::SerializationService &serializationService;
                std::vector<Item> deserializedEntries;

            private:
                // prevent copy operations
                LazyEntryArrayImpl(const LazyEntryArrayImpl &rhs);

                LazyEntryArrayImpl &operator=(const LazyEntryArrayImpl &rhs);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_LAZYENTRYARRAYIMPL_H_

