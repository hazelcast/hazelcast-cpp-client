/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

namespace hazelcast {
namespace client {
namespace config {

/**
 * Contains client configurations for TPC.
 * <p>
 * TPC is the next generation Hazelcast that uses thread-per-core model.
 * <p>
 * TPC-aware clients will maintain connections to all cores of all cluster
 * members. The client will route partition-specific invocations to the
 * correct core of the correct member in the best effort basis.
 *
 * @since 5.3
 */
class HAZELCAST_API client_tpc_config
{
public:
    client_tpc_config();

    /**
     * Returns if the TPC-aware mode is enabled.
     *
     * @return {@code true} if the TPC-aware mode is enabled, {@code false}
     * otherwise.
     * @since 5.3
     */
    bool is_enabled() const;

    /**
     * Enables or disables the TPC-aware mode.
     * <p>
     * When enabled, the configuration option set by the
     * {@link client_network_config#set_smart_routing(boolean)} is ignored.
     *
     * @param enabled flag to enable or disable TPC-aware mode
     * @return this configuration for chaining.
     * @since 5.3
     */
    client_tpc_config& set_enabled(bool enabled);

private:
    bool enabled_;
};

std::ostream HAZELCAST_API&
operator<<(std::ostream& os, const client_tpc_config& cfg);

} // namespace config
} // namespace client
} // namespace hazelcast