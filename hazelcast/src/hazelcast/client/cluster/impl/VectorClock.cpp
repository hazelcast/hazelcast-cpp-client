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
#include <utility>
#include <boost/foreach.hpp>

#include "hazelcast/client/cluster/impl/VectorClock.h"

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace impl {

                VectorClock::VectorClock() {}

                VectorClock::VectorClock(const VectorClock::TimestampVector &replicaLogicalTimestamps) {
                    BOOST_FOREACH(const TimestampVector::value_type &replicaTimestamp, replicaLogicalTimestamps) {
                                    replicaTimestamps[replicaTimestamp.first] = replicaTimestamp.second;
                                }
                }

                VectorClock::TimestampVector VectorClock::entrySet() {
                    TimestampVector result;
                    BOOST_FOREACH(const TimestampMap::value_type &entry, replicaTimestamps) {
                                    result.push_back(std::make_pair(entry.first, entry.second));
                    }

                    return result;
                }

                bool VectorClock::isAfter(VectorClock &other) {
                    bool anyTimestampGreater = false;
                    BOOST_FOREACH(const TimestampMap::value_type &otherEntry, other.replicaTimestamps) {
                        const std::string &replicaId = otherEntry.first;
                                    int64_t otherReplicaTimestamp = otherEntry.second;
                                    std::pair<bool, int64_t> localReplicaTimestamp = getTimestampForReplica(replicaId);

                                    if (!localReplicaTimestamp.first ||
                                        localReplicaTimestamp.second < otherReplicaTimestamp) {
                            return false;
                                    } else if (localReplicaTimestamp.second > otherReplicaTimestamp) {
                            anyTimestampGreater = true;
                        }
                    }
                    // there is at least one local timestamp greater or local vector clock has additional timestamps
                    return anyTimestampGreater ||  other.replicaTimestamps.size() < replicaTimestamps.size();
                }

                std::pair<bool, int64_t> VectorClock::getTimestampForReplica(const std::string &replicaId) {
                    if (replicaTimestamps.count(replicaId) == 0) {
                        return std::make_pair(false, -1);
                    }
                    return std::make_pair(true, replicaTimestamps[replicaId]);
                }

            }
        }
    }
}
