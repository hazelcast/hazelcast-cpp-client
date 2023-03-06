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

#include <boost/thread/future.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/serialization/pimpl/compact/schema.h"
#include "hazelcast/logger.h"

namespace hazelcast {
namespace client {

namespace protocol {
class ClientMessage;
}

namespace spi {
class ClientContext;
}
namespace serialization {
namespace pimpl {

/**
 * Service to put and get metadata to cluster.
 */
class HAZELCAST_API default_schema_service
{
public:
    static constexpr const char* SERVICE_NAME = "schema-service";
    /**
     * Maximum number of attempts for schema replication process.
     */
    static constexpr const char* MAX_PUT_RETRY_COUNT =
      "hazelcast.client.schema.max.put.retry.count";
    static constexpr const char* MAX_PUT_RETRY_COUNT_DEFAULT = "100";

    default_schema_service(spi::ClientContext&);
    default_schema_service(const default_schema_service&) = delete;
    default_schema_service& operator=(const default_schema_service&) = delete;

    /**
     * Gets the schema with the given id either by
     * <ul>
     *     <li>returning it directly from the local registry, if it exists.</li>
     *     <li>searching the cluster.</li>
     * </ul>
     */
    std::shared_ptr<schema> get(int64_t schemaId);

    /**
     * Replicates schema on the cluster
     */
    void replicate_schema_in_cluster(schema);

    bool is_schema_replicated(const schema&);

    /**
     * Replicates all schemas on cluster
     */
    void replicate_all_schemas();

    /**
     * Check whether any schemas exist in cache
     */
    bool has_any_schemas() const;

private:
    void put_if_absent(schema);

    /**
     * Decodes response of send schema request
     */
    std::unordered_set<boost::uuids::uuid, boost::hash<boost::uuids::uuid>>
    send_schema_response_decode(protocol::ClientMessage&);

    int retry_pause_millis_;
    int max_put_retry_count_;
    spi::ClientContext& context_;
    util::SynchronizedMap<int64_t, schema> replicateds_;
};

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast