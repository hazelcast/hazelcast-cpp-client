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
#ifndef HAZELCAST_CLIENT_QUERY_ENTRYCOMPARATOR_H_
#define HAZELCAST_CLIENT_QUERY_ENTRYCOMPARATOR_H_

#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/util/Comparator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace query {
            template <typename K, typename V>
            class EntryComparator : public serialization::IdentifiedDataSerializable,
                                    public util::Comparator<std::pair<const K *, const V *> > {
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_QUERY_ENTRYCOMPARATOR_H_ */
