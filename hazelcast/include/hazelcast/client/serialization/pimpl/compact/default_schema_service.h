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

#include <boost/thread/future.hpp>

#include "hazelcast/util/export.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/serialization/pimpl/compact/schema.h"

namespace hazelcast {
namespace client {

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

    /**
     * Gets the schema with the given id either by
     * <ul>
     *     <li>returning it directly from the local registry, if it exists.</li>
     *     <li>searching the cluster.</li>
     * </ul>
     */
    schema get(int64_t schemaId);

    /**
     * Replicates schema on the cluster
     */
    boost::future<void> replicate_schema(schema);

    bool is_schema_replicated(const schema&);

private:
    boost::future<void> replicate_schema_attempt(schema, int attempts = 0);

    int retry_pause_millis_;
    int max_put_retry_count_;
    spi::ClientContext& context_;
    util::SynchronizedMap<int64_t, schema> replicateds_;
};

} // namespace pimpl
} // namespace serialization
} // namespace client
} // namespace hazelcast