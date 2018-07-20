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

#ifndef HAZELCAST_CLIENT_CLUSTER_IMPL_VECTORCLOCK_H
#define HAZELCAST_CLIENT_CLUSTER_IMPL_VECTORCLOCK_H

#include <string>
#include <stdint.h>

#include "hazelcast/util/SynchronizedMap.h"

namespace hazelcast {
    namespace client {
        namespace cluster {
            namespace impl {
                /**
                 * Vector clock consisting of distinct replica logical clocks.
                 * <p>
                 * See https://en.wikipedia.org/wiki/Vector_clock
                 * The vector clock may be read from different thread but concurrent
                 * updates must be synchronized externally. There is no guarantee for
                 * concurrent updates.
                 */
                class VectorClock {
                public:
                    VectorClock();

                    /**
                     * Returns logical timestamp for given {@code replicaId}.
                     * This method may be called from different threads and the result reflects
                     * the latest update on the vector clock.
                     */
                    boost::shared_ptr<int64_t> getTimestampForReplica(const std::string &replicaId);

                    /**
                     * Sets the logical timestamp for the given {@code replicaId}.
                     * This method is not thread safe and concurrent access must be synchronized
                     * externally.
                     */
                    void setReplicaTimestamp(const std::string &replicaId, int64_t timestamp);

                    /** Returns a set of replica logical timestamps for this vector clock. */
                    std::vector<std::pair<std::string, int64_t> > entrySet();

                    /**
                     * Returns {@code true} if this vector clock is causally strictly after the
                     * provided vector clock. This means that it the provided clock is neither
                     * equal to, greater than or concurrent to this vector clock.
                     * This method may be called from different threads and the result reflects
                     * the latest update on the vector clock.
                     */
                    bool isAfter(VectorClock &other);

                private:
                    util::SynchronizedMap<std::string, int64_t> replicaTimestamps;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_CLUSTER_IMPL_VECTORCLOCK_H

