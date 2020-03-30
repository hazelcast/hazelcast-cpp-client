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
#ifndef HAZELCAST_CLIENT_SPI_CLIENTINVOCATIONSERVICE_H_
#define HAZELCAST_CLIENT_SPI_CLIENTINVOCATIONSERVICE_H_

#include <stdint.h>
#include <memory>
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            /**
             * Invocation service for Hazelcast clients.
             *
             * Allows remote invocations on different targets like {@link ClientConnection},
             * partition owners or {@link Address} based targets.
             */
            class HAZELCAST_API ClientInvocationService : public protocol::IMessageHandler {
            public:
                virtual ~ClientInvocationService() {
                }

                virtual void invokeOnConnection(std::shared_ptr<impl::ClientInvocation> invocation,
                                                std::shared_ptr<connection::Connection> connection) = 0;

                virtual void
                invokeOnPartitionOwner(std::shared_ptr<impl::ClientInvocation> invocation, int partitionId)  = 0;

                virtual void invokeOnRandomTarget(std::shared_ptr<impl::ClientInvocation> invocation)  = 0;

                virtual void invokeOnTarget(std::shared_ptr<impl::ClientInvocation> invocation,
                                            const std::shared_ptr<Address> &target)  = 0;

                virtual bool isRedoOperation() = 0;

                virtual int64_t getInvocationTimeoutMillis() const = 0;

                virtual int64_t getInvocationRetryPauseMillis() const = 0;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__InvocationService_H_

