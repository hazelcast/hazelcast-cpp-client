/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 19/08/14.
//

#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/connection/OwnerConnectionFuture.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ConnectionManager.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            OwnerConnectionFuture::OwnerConnectionFuture(spi::ClientContext& clientContext)
            : clientContext(clientContext) {

            }

            void OwnerConnectionFuture::markAsClosed() {
                ownerConnectionPtr.reset();
            }

            boost::shared_ptr<Connection> OwnerConnectionFuture::createNew(const Address& address) {
                ownerConnectionPtr = clientContext.getConnectionManager().connectTo(address, true);
                ownerConnectionPtr->setAsOwnerConnection(true);
                return ownerConnectionPtr;
            }

            boost::shared_ptr<Connection> OwnerConnectionFuture::getOrWaitForCreation() {
                boost::shared_ptr<Connection> currentOwnerConnection = ownerConnectionPtr;
                if (currentOwnerConnection.get() != NULL) {
                    return currentOwnerConnection;
                }
                ClientConfig& config = clientContext.getClientConfig();
                int tryCount = 2 * config.getAttemptPeriod() * config.getConnectionAttemptLimit() / 1000;
				
                while (currentOwnerConnection.get() == NULL) {
                    currentOwnerConnection = ownerConnectionPtr;
                    util::sleep(1);
                    if (--tryCount == 0) {
                        throw exception::IOException("ConnectionManager", "Wait for owner connection is timed out");
                    }
                }
                return currentOwnerConnection;
            }


            void OwnerConnectionFuture::closeIfAddressMatches(const Address& address) {
                boost::shared_ptr<Connection> currentOwnerConnection = ownerConnectionPtr;

                if (currentOwnerConnection.get() == NULL || !currentOwnerConnection->live) {
                    return;
                }

                if (currentOwnerConnection->getRemoteEndpoint() == address) {
                    close();
                }
            }

            void OwnerConnectionFuture::close() {
                boost::shared_ptr<Connection> currentOwnerConnection = ownerConnectionPtr;
                if (currentOwnerConnection.get() == NULL) {
                    return;
                }

                std::stringstream message;
                message << "Closing owner connection to " << currentOwnerConnection->getRemoteEndpoint();
                util::ILogger::getLogger().finest(message.str());
                util::IOUtil::closeResource(currentOwnerConnection.get());
                markAsClosed();
            }
        }
    }
}

