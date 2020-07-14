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

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/impl/AbstractClientInvocationService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class LoadBalancer;

        namespace spi {
            namespace impl {
                class HAZELCAST_API NonSmartClientInvocationService : public AbstractClientInvocationService {
                public:
                    NonSmartClientInvocationService(ClientContext &client);

                    void invokeOnConnection(std::shared_ptr<impl::ClientInvocation> invocation,
                                            std::shared_ptr<connection::Connection> connection) override;

                    void invokeOnPartitionOwner(std::shared_ptr<impl::ClientInvocation> invocation,
                                                int partitionId) override;

                    void invokeOnRandomTarget(std::shared_ptr<impl::ClientInvocation> invocation) override;

                    void invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                        const std::shared_ptr<Address> &target) override;

                    std::shared_ptr<connection::Connection> getOwnerConnection();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


