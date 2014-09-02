//
// Created by sancar koyunlu on 19/08/14.
//

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
                Connection *clientConnection = clientContext.getConnectionManager().connectTo(address, true);
                clientConnection->setAsOwnerConnection(true);
                ownerConnectionPtr.reset(clientConnection);
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

