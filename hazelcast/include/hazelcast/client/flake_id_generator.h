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
#pragma once

#include "hazelcast/util/export.h"
#include "hazelcast/client/proxy/flake_id_generator_impl.h"

namespace hazelcast {
    namespace client {

        /**
         * A cluster-wide unique ID generator. Generated IDs are {@code int64_t} primitive values
         * and are k-ordered (roughly ordered). IDs are in the range from {@code 0} to {@code
         * INT64_MAX}.
         * <p>
         * The IDs contain timestamp component and a node ID component, which is assigned when the member
         * joins the cluster. This allows the IDs to be ordered and unique without any coordination between
         * members, which makes the generator safe even in split-brain scenario (for caveats,
         * {@link ClusterService#getMemberListJoinVersion() see here}).
         * <p>
         * Timestamp component is in milliseconds since 1.1.2018, 0:00 UTC and has 41 bits. This caps
         * the useful lifespan of the generator to little less than 70 years (until ~2088). The sequence component
         * is 6 bits. If more than 64 IDs are requested in single millisecond, IDs will gracefully overflow to the next
         * millisecond and uniqueness is guaranteed in this case. The implementation does not allow overflowing
         * by more than 15 seconds, if IDs are requested at higher rate, the call will block. Note, however, that
         * clients are able to generate even faster because each call goes to a different (random) member and
         * the 64 IDs/ms limit is for single member.
         *
         * <h4>Node ID overflow</h4>
         * Node ID component of the ID has 16 bits. Members with member list join version higher than
         * 2^16 won't be able to generate IDs, but functionality will be preserved by forwarding to another
         * member. It is possible to generate IDs on any member or client as long as there is at least one
         * member with join version smaller than 2^16 in the cluster. The remedy is to restart the cluster:
         * nodeId will be assigned from zero again. Uniqueness after the restart will be preserved thanks to
         * the timestamp component.
         *
         * @since 3.10.1
         */
    class HAZELCAST_API flake_id_generator : public proxy::flake_id_generator_impl {
        friend class spi::ProxyManager;
        public:
            static constexpr const char *SERVICE_NAME = "hz:impl:flakeIdGeneratorService";

        private:
            flake_id_generator(const std::string &object_name, spi::ClientContext *context);
        };
    }
}
