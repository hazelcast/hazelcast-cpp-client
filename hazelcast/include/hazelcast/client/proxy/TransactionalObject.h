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

#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/proxy/SerializingProxy.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }

        namespace proxy {
            class HAZELCAST_API TransactionalObject : public proxy::SerializingProxy {
            public:
                TransactionalObject(const std::string &serviceName, const std::string &objectName,
                                    txn::TransactionProxy &context);

                virtual ~TransactionalObject();

                const std::string &getServiceName();

                const std::string &getName();

                boost::future<void> destroy();

            protected:
                virtual void onDestroy();

                boost::uuids::uuid getTransactionId() const;

                std::chrono::steady_clock::duration getTimeout() const;

                const std::string serviceName;
                const std::string name;
                txn::TransactionProxy &context;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

