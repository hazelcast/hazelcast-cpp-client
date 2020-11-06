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

#include <memory>

#include "hazelcast/util/hazelcast_dll.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/address.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class load_balancer;

        namespace connection {
            class Connection;
        }
        namespace spi {
            class ClientContext;
            namespace impl {
                class HAZELCAST_API ClientTransactionManagerServiceImpl {
                public:
                    ClientTransactionManagerServiceImpl(ClientContext &client);

                    ClientContext &get_client() const;

                    std::shared_ptr<connection::Connection> connect();

                private:
                    ClientContext &client_;

                    std::exception_ptr
                    new_operation_timeout_exception(std::exception_ptr cause, std::chrono::milliseconds invocation_timeout,
                                                 std::chrono::steady_clock::time_point start_time);

                    void throw_exception(bool smart_routing);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


