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

#ifndef HAZELCAST_CLIENT_TXN_CLIENTTRANSACTIONUTIL_H_
#define HAZELCAST_CLIENT_TXN_CLIENTTRANSACTIONUTIL_H_

#include <memory>
#include <string>

#include <boost/shared_ptr.hpp>

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
            class ClientTransactionUtil {
            public:
                /**
                 * Handles the invocation exception for transactions so that users will not see internal exceptions.
                 * <p>
                 * More specifically IOException, because in case of a IO problem in ClientInvocation that send to a connection
                 * sends IOException to user. This wraps that exception into a TransactionException.
                 */
                static boost::shared_ptr<protocol::ClientMessage>
                invoke(std::auto_ptr<protocol::ClientMessage> &request, const std::string &objectName,
                       spi::ClientContext &client, const boost::shared_ptr<connection::Connection> &connection);
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_TXN_CLIENTTRANSACTIONUTIL_H_

