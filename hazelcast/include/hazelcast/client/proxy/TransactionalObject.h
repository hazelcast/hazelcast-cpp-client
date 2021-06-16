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

#include "hazelcast/client/txn/TransactionProxy.h"
#include "hazelcast/client/proxy/SerializingProxy.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace connection {
class Connection;
}

namespace proxy {
class HAZELCAST_API TransactionalObject : public proxy::SerializingProxy
{
public:
    TransactionalObject(const std::string& service_name,
                        const std::string& object_name,
                        txn::TransactionProxy& context);

    virtual ~TransactionalObject();

    const std::string& get_service_name();

    const std::string& get_name();

    boost::future<void> destroy();

protected:
    virtual void on_destroy();

    boost::uuids::uuid get_transaction_id() const;

    std::chrono::milliseconds get_timeout() const;

    const std::string service_name_;
    const std::string name_;
    txn::TransactionProxy& context_;
};
} // namespace proxy
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
