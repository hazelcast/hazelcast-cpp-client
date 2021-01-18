/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <string>
#include <stdint.h>
#include <unordered_map>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/functional/hash.hpp>

namespace hazelcast {
    namespace client {
        namespace impl {

            /**
                 * Vector clock consisting of distinct replica logical clocks.
                 * <p>
                 * See https://en.wikipedia.org/wiki/Vector_clock
                 * There is no guarantee for concurrent updates.
                 */
            class vector_clock {
            public:
                typedef std::vector<std::pair<boost::uuids::uuid, int64_t> > timestamp_vector;
                typedef std::unordered_map<boost::uuids::uuid, int64_t, boost::hash<boost::uuids::uuid>> timestamp_map;

                vector_clock();

                vector_clock(const timestamp_vector &replica_logical_timestamps);

                /** Returns a set of replica logical timestamps for this vector clock. */
                timestamp_vector entry_set();

                /**
                 * Returns {@code true} if this vector clock is causally strictly after the
                 * provided vector clock. This means that it the provided clock is neither
                 * equal to, greater than or concurrent to this vector clock.
                 */
                bool is_after(vector_clock &other);

            private:
                /**
                 * Returns logical timestamp for given {@code replicaId}.
                 * @return false for the pair.first if timestamp does not exist for replicaId,
                 * otherwise returns true for pair.first and the timestamp of the replica as the pair.second.
                 */
                std::pair<bool, int64_t> get_timestamp_for_replica(boost::uuids::uuid replica_id);

                timestamp_map replica_timestamps_;
                vector_clock::timestamp_vector replica_timestamp_entries_;
            };
        }
    }
}



