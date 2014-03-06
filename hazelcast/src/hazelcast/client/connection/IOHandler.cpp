//
// Created by sancar koyunlu on 30/12/13.
//

#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/client/connection/Connection.h"
#include "ILogger.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            IOHandler::IOHandler(Connection &connection, IOSelector &ioListener)
            :connection(connection)
            , ioListener(ioListener) {

            }

            void IOHandler::registerSocket() {
                ioListener.addTask(this);
                ioListener.wakeUp();
            }

            void IOHandler::registerHandler() {
                if (!connection.live)
                    return;
                Socket const &socket = connection.getSocket();
                ioListener.addSocket(socket);
            }

            IOHandler::~IOHandler() {

            }

            void IOHandler::handleSocketException(const std::string &message) {
                ioListener.removeSocket(connection.getSocket());
                connection.close();
                std::stringstream warningStr;
                Address const &address = connection.getRemoteEndpoint();
                (warningStr << " Closing socket to endpoint " << address.getHost() << ":" << address.getPort()
                        << ", Cause:" << message << std::endl);
                util::ILogger::warning(warningStr.str());
            }
        }
    }
}
