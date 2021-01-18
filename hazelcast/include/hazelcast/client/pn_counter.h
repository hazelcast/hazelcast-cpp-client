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

#include "hazelcast/client/proxy/PNCounterImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * PN (Positive-Negative) CRDT counter.
         * <p>
         * The counter supports adding and subtracting values as well as
         * retrieving the current counter value.
         * Each replica of this counter can perform operations locally without
         * coordination with the other replicas, thus increasing availability.
         * The counter guarantees that whenever two nodes have received the
         * same set of updates, possibly in a different order, their state is
         * identical, and any conflicting updates are merged automatically.
         * If no new updates are made to the shared state, all nodes that can
         * communicate will eventually have the same data.
         * <p>
         * The updates to this counter are applied locally when invoked on a
         * CRDT replica. A replica can be any hazelcast instance which is not a
         * client or a lite member. The number of replicas in the cluster is
         * determined by the {@link PNCounterConfig#getReplicaCount()} server configuration
         * value.
         * <p>
         * When invoking updates from non-replica instance, the invocation is remote.
         * This may lead to indeterminate state - the update may be applied but the
         * response has not been received. In this case, the caller will be notified
         * with a {@link com.hazelcast.spi.exception.target_disconnected}
         * when invoking from a client or a
         * {@link com.hazelcast.core.member_left} when invoked from a member.
         * <p>
         * The read and write methods provide monotonic read and RYW (read-your-write)
         * guarantees. These guarantees are session guarantees which means that if
         * no replica with the previously observed state is reachable, the session
         * guarantees are lost and the method invocation will throw a
         * {@link consistency_lost}. This does not mean
         * that an update is lost. All of the updates are part of some replica and
         * will be eventually reflected in the state of all other replicas. This
         * exception just means that you cannot observe your own writes because
         * all replicas that contain your updates are currently unreachable.
         * After you have received a {@link consistency_lost}, you can either
         * wait for a sufficiently up-to-date replica to become reachable in which
         * case the session can be continued or you can reset the session by calling
         * the {@link #reset()} method. If you have called the {@link #reset()} method,
         * a new session is started with the next invocation to a CRDT replica.
         * <p>
         * <b>NOTE:</b>
         * The CRDT state is kept entirely on non-lite (data) members. If there
         * aren't any and the methods here are invoked on a lite member, they will
         * fail with an {@link no_data_member_in_cluster}.
         *
         * @since 3.10
         */
        class HAZELCAST_API pn_counter : public proxy::PNCounterImpl {
            friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:PNCounterService";
        private:
            pn_counter(const std::string &object_name, spi::ClientContext *context) : PNCounterImpl(
                    SERVICE_NAME, object_name, context) {}
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
