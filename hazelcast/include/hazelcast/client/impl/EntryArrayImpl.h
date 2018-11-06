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
#ifndef HAZELCAST_CLIENT_IMPL_ENTRYARRAYIMPL_H_
#define HAZELCAST_CLIENT_IMPL_ENTRYARRAYIMPL_H_

#include "hazelcast/client/impl/LazyEntryArrayImpl.h"
#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/EntryArray.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            template<typename K, typename V>
            class EntryArrayImpl : public EntryArray<K, V>, public LazyEntryArrayImpl<K, V> {
            public:
                EntryArrayImpl(
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries,
                        serialization::pimpl::SerializationService &service) : LazyEntryArrayImpl<K, V>(entries, service) {
                }

                /**
                 * Modifies the array. Do not use the array after this call but just use the new object
                 */
                EntryArrayImpl(EntryArrayImpl &array, size_t begin, size_t end) : LazyEntryArrayImpl<K, V>(array, begin, end) {
                    if (end < begin) {
                        throw exception::IllegalArgumentException("EntryArrayImpl", "end should be greater than begin!");
                    }

                    size_t size = array.size();
                    if (end > size) {
                        throw exception::IllegalArgumentException("EntryArrayImpl", "end should not be greater than array size!");
                    }

                    // make sure that items are deserialized
                    for (size_t i = 0; i < size; ++i) {
                        array[i];
                    }

                    for (size_t i = begin; i < end; ++i) {
                        typename LazyEntryArrayImpl<K, V>::Item &item = array.deserializedEntries[i];
                        LazyEntryArrayImpl<K, V>::dataEntries.push_back(*item.data);
                        LazyEntryArrayImpl<K, V>::deserializedEntries.push_back(item);
                        // invalidate the entry at the original array
                        item.isValueDeserialized = false;
                        item.isKeyDeserialized = false;
                    }
                }

                /**
                 * Sorts the entries
                 */
                void sort(query::IterationType iterationType, const util::Comparator<std::pair<const K *, const V *> > *comparator) {
                    // make sure that all entries are deserialized, we do this since the std::sort requires that we use
                    // const methods when writing the < operator
                    for (typename std::vector<typename LazyEntryArrayImpl<K, V>::Item>::iterator it = LazyEntryArrayImpl<K, V>::deserializedEntries.begin();
                         it != LazyEntryArrayImpl<K, V>::deserializedEntries.end(); ++it) {
                        it->getKey();
                        it->getValue();
                        it->comparator = comparator;
                        it->type = iterationType;
                    }
                    std::sort(LazyEntryArrayImpl<K, V>::deserializedEntries.begin(), LazyEntryArrayImpl<K, V>::deserializedEntries.end());
                }
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_ENTRYARRAYIMPL_H_

