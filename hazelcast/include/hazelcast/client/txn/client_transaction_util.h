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

#include <memory>
#include <string>

#include <boost/thread/future.hpp>

#include "hazelcast/util/exception_util.h"

namespace hazelcast {
namespace client {
namespace protocol {
class ClientMessage;
}

namespace spi {
class ClientContext;
}

namespace connection {
class Connection;
}
namespace txn {
/**
 * Contains static method that is used from client transaction classes.
 */
class client_transaction_util
{
public:
    /**
     * Handles the invocation exception for transactions so that users will not
     * see internal exceptions. <p> More specifically io , because in case of a
     * IO problem in ClientInvocation that send to a connection sends io to
     * user. This wraps that exception into a transaction_exception.
     */
    static boost::future<protocol::ClientMessage> invoke(
      protocol::ClientMessage& request,
      const std::string& object_name,
      spi::ClientContext& client,
      const std::shared_ptr<connection::Connection>& connection);

    static const std::shared_ptr<
      util::exception_util::runtime_exception_factory>&
    transaction_exception_factory();

private:
    class transaction_exception_factory
      : public util::exception_util::runtime_exception_factory
    {
    public:
        void rethrow(std::exception_ptr throwable,
                     const std::string& message) override;
    };

    static const std::shared_ptr<
      util::exception_util::runtime_exception_factory>
      exceptionFactory;
};
} // namespace txn
} // namespace client
} // namespace hazelcast
