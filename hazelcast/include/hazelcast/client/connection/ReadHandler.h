//
// Created by sancar koyunlu on 24/12/13.
//


#ifndef HAZELCAST_ReadHandler
#define HAZELCAST_ReadHandler

#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/connection/IOHandler.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Packet;

                class PortableContext;
            }
        }
        namespace spi {
            class ClientContext;
        }
        namespace connection {
            class Connection;

            class ConnectionManager;

            class InSelector;

            class ReadHandler : public IOHandler {
            public:
                ReadHandler(Connection &connection, InSelector &iListener, size_t bufferSize, spi::ClientContext& clientContext);

                ~ReadHandler();

                void handle();

                void run();

            private:
                char* buffer;
                util::ByteBuffer byteBuffer;
                serialization::pimpl::Packet *lastData;
                spi::ClientContext& clientContext;

                serialization::pimpl::PortableContext& getPortableContext();


            };
        }
    }
}

#endif //HAZELCAST_ReadHandler

