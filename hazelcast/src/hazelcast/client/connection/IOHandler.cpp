//
// Created by sancar koyunlu on 30/12/13.
//

#include "hazelcast/client/connection/IOHandler.h"
#include "hazelcast/client/connection/IOListener.h"
#include "hazelcast/client/connection/Connection.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            IOHandler::IOHandler(Connection &connection, IOListener &ioListener)
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
                hazelcast::client::Socket const &socket = connection.getSocket();
                ioListener.addSocket(socket);
                ioListener.addHandler(socket.getSocketId(), this);
            }

            IOHandler::~IOHandler() {

            }

            void IOHandler::handleSocketException(const std::string &message) {
                ioListener.removeSocket(connection.getSocket());
                connection.close();
                (std::cerr << message << std::endl);
            }
        }
    }
}
