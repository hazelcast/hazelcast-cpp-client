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
#ifndef HAZELCAST_CLIENT_ENTRYARRAY_H_
#define HAZELCAST_CLIENT_ENTRYARRAY_H_

#include "hazelcast/util/Comparator.h"
#include "hazelcast/client/query/PagingPredicate.h"
#include "hazelcast/client/LazyEntryArray.h"

namespace hazelcast {
    namespace client {
        template<typename K, typename V>
        class EntryArray : virtual public LazyEntryArray<K, V> {
        public:
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

