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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTTRANSACTIONMANAGERIMPL_H_
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTTRANSACTIONMANAGERIMPL_H_

#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/Address.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class LoadBalancer;

        namespace connection {
            class Connection;
        }
        namespace spi {
            class ClientContext;
            namespace impl {
                class HAZELCAST_API ClientTransactionManagerServiceImpl {
                public:
                    ClientTransactionManagerServiceImpl(ClientContext &client, LoadBalancer &loadBalancer);

                    ClientContext &getClient() const;

                    std::shared_ptr<connection::Connection> connect();

                private:
                    ClientContext &client;
                    LoadBalancer &loadBalancer;

                    std::shared_ptr<connection::Connection> tryConnectSmart();

                    Address getRandomAddress();

                    std::shared_ptr<connection::Connection> tryConnectUnisocket();

                    std::shared_ptr<connection::Connection> throwException(bool smartRouting);

                    std::exception_ptr
                    newOperationTimeoutException(std::exception_ptr cause, int64_t invocationTimeoutMillis,
                                                 int64_t startTimeMillis);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_CLIENTTRANSACTIONMANAGERIMPL_H_
