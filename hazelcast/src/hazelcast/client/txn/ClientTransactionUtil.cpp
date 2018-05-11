/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/txn/ClientTransactionUtil.h"
#include "hazelcast/client/spi/impl/ClientInvocation.h"

namespace hazelcast {
    namespace client {
        namespace txn {

            boost::shared_ptr<protocol::ClientMessage>
            ClientTransactionUtil::invoke(std::auto_ptr<protocol::ClientMessage> &request,
                                          const std::string &objectName,
                                          spi::ClientContext &client,
                                          const boost::shared_ptr<connection::Connection> &connection) {
                try {
                    boost::shared_ptr<spi::impl::ClientInvocation> clientInvocation = spi::impl::ClientInvocation::create(
                            client, request, objectName, connection);
                    boost::shared_ptr<spi::impl::ClientInvocationFuture> future = clientInvocation->invoke();
                    return future->get();
                } catch (exception::IException &e) {
                    throw exception::TransactionException("ClientTransactionUtil::invoke", "Transaction exception.",
                                                          boost::shared_ptr<exception::IException>(e.clone()));
                }
            }
        }
    }
}
