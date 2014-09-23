//
// Created by sancar koyunlu on 30/12/13.
//

#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            IOHandler::IOHandler(Connection& connection, IOSelector& ioSelector)
            : ioSelector(ioSelector)
            , connection(connection) {

            }

            void IOHandler::registerSocket() {
                ioSelector.addTask(this);
                ioSelector.wakeUp();
            }

            void IOHandler::registerHandler() {
                if (!connection.live)
                    return;
                Socket const& socket = connection.getSocket();
                ioSelector.addSocket(socket);
            }

            void IOHandler::deRegisterSocket() {
                ioSelector.removeSocket(connection.getSocket());
            }

            IOHandler::~IOHandler() {

            }

            void IOHandler::handleSocketException(const std::string& message) {
                std::stringstream warningStr;
                Address const& address = connection.getRemoteEndpoint();
                (warningStr << " Closing socket to endpoint " << address.getHost() << ":" << address.getPort()
                << ", Cause:" << message);
                util::ILogger::getLogger().getLogger().warning(warningStr.str());
                connection.close();
            }
        }
    }
}

