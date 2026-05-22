/*
 * Copyright (c) 2008-2026, Hazelcast, Inc. All Rights Reserved.
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

#include <memory>

#include "hazelcast/client/impl/auto_batcher.h"
#include "hazelcast/client/impl/id_batch.h"
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/spi/impl/ClientExecutionServiceImpl.h"

namespace hazelcast {
namespace client {
namespace proxy {
class HAZELCAST_API flake_id_generator_impl : public ProxyImpl
{
public:
    /**
     * Generates and returns a cluster-wide unique ID.
     * <p>
     * Operation on member is always local, if the member has valid node ID,
     * otherwise it's remote. On client, this method goes to a random member and
     * gets a batch of IDs, which will then be returned locally for limited
     * time. The pre-fetch size and the validity time can be configured for each
     * client and member, see {@code ClientConfig.addFlakeIdGeneratorConfig()}
     * for client config. <p> <b>Note:</b> Values returned from this method may
     * be not strictly ordered.
     *
     * @return new cluster-wide unique ID
     *
     * @throws node_id_out_of_range if node ID for all members in the cluster is
     * out of valid range. See "Node ID overflow" in {@link FlakeIdGenerator
     * class documentation} for more details.
     */
    boost::future<int64_t> new_id();

protected:
    flake_id_generator_impl(const std::string& service_name,
                            const std::string& object_name,
                            spi::ClientContext* context);

private:
    boost::future<impl::id_batch> new_id_batch(int32_t size);

    std::shared_ptr<spi::impl::ClientExecutionServiceImpl> execution_service_;
    impl::auto_batcher batcher_;
};

} // namespace proxy
} // namespace client
} // namespace hazelcast
