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
#include <boost/foreach.hpp>
#include <set>

#include "hazelcast/client/cluster/impl/VectorClock.h"
#include "hazelcast/client/cluster/impl/ClusterDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace impl {

                VectorClock::VectorClock() {}

                typedef std::vector<std::pair<std::string, boost::shared_ptr<int64_t> > > TimestampVector;

                std::vector<std::pair<std::string, int64_t> > VectorClock::entrySet() {
                    TimestampVector entries = replicaTimestamps.entrySet();
                    std::vector<std::pair<std::string, int64_t> > result;
                    BOOST_FOREACH(const TimestampVector::value_type &entry , entries) {
                        result.push_back(std::make_pair(entry.first, *entry.second));
                    }

                    return result;
                }

                void VectorClock::setReplicaTimestamp(const std::string &replicaId, int64_t timestamp) {
                    replicaTimestamps.put(replicaId, boost::shared_ptr<int64_t >(new int64_t(timestamp)));
                }

                bool VectorClock::isAfter(VectorClock &other) {
                    bool anyTimestampGreater = false;
                    BOOST_FOREACH (const TimestampVector::value_type &otherEntry , other.replicaTimestamps.entrySet()) {
                        const std::string &replicaId = otherEntry.first;
                        const boost::shared_ptr<int64_t> &otherReplicaTimestamp = otherEntry.second;
                        boost::shared_ptr<int64_t> localReplicaTimestamp = getTimestampForReplica(replicaId);

                        if (localReplicaTimestamp.get() == NULL || localReplicaTimestamp < otherReplicaTimestamp) {
                            return false;
                        } else if (*localReplicaTimestamp > *otherReplicaTimestamp) {
                            anyTimestampGreater = true;
                        }
                    }
                    // there is at least one local timestamp greater or local vector clock has additional timestamps
                    return anyTimestampGreater || !containsAll(other.replicaTimestamps.keys(), replicaTimestamps.keys());
                }

                boost::shared_ptr<int64_t> VectorClock::getTimestampForReplica(const std::string &replicaId) {
                    return replicaTimestamps.get(replicaId);
                }

                bool VectorClock::containsAll(const std::vector<std::string> &replicas, const std::vector<std::string> &searchedReplicas) {
                    std::set<std::string> currentReplicas;
                    BOOST_FOREACH(const std::string &replicaId , replicas) {
                        currentReplicas.insert(replicaId);
                    }
                    BOOST_FOREACH(const std::string &replicaId , searchedReplicas) {
                        if (currentReplicas.find(replicaId) == currentReplicas.end()) {
                            return false;
                        }
                    }
                    return true;
                }

                int VectorClock::getFactoryId() const {
                    return F_ID;
                }

                int VectorClock::getClassId() const {
                    return VECTOR_CLOCK;
                }

                void VectorClock::writeData(serialization::ObjectDataOutput &writer) const {
                    // should not be executed since this is a client
                    assert(0);
                }

                void VectorClock::readData(serialization::ObjectDataInput &in) {
                    int32_t stateSize = in.readInt();
                    for (int32_t i = 0; i < stateSize; i++) {
                        std::auto_ptr<std::string> replicaId = in.readUTF();
                        int64_t timestamp = in.readLong();
                        replicaTimestamps.put(*replicaId, boost::shared_ptr<int64_t>(new int64_t(timestamp)));
                    }
                }
            }
        }
    }
}
