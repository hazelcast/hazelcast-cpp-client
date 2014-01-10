//
// Created by sancar koyunlu on 28/12/13.
//


#ifndef HAZELCAST_IOHandler
#define HAZELCAST_IOHandler

#include "hazelcast/client/connection/ListenerTask.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace connection {
            class IOListener;

            class Connection;

            class HAZELCAST_API IOHandler : public ListenerTask {
            public:

                IOHandler(Connection &connection, IOListener &ioListener);

                ~IOHandler();

                virtual void handle() = 0;

                void handleSocketException(const std::string& message);

                void registerSocket();

            protected:
                IOListener &ioListener;

                Connection &connection;

                void registerHandler();

            };
        }
    }
}

#endif //HAZELCAST_IOHandler
