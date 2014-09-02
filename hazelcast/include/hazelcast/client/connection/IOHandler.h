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
            class IOSelector;

            class Connection;

            class HAZELCAST_API IOHandler : public ListenerTask {
            public:

                IOHandler(Connection &connection, IOSelector & ioSelector);

                virtual ~IOHandler();

                virtual void handle() = 0;

                void registerSocket();

                /* should be called from registered io thread */
                void deRegisterSocket();

            protected:
                IOSelector & ioSelector;
                Connection &connection;

                void registerHandler();

                void handleSocketException(const std::string& message);

            };
        }
    }
}

#endif //HAZELCAST_IOHandler

