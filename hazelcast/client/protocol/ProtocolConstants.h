#ifndef HAZELCAST_PROTOCOL_CONSTANTS
#define HAZELCAST_PROTOCOL_CONSTANTS

#include <vector>

namespace hazelcast {
    namespace client {
        typedef unsigned char byte;
        namespace protocol {
            namespace ProtocolConstants {
                int const CLIENT_MAP_FACTORY_ID = -10;
                int const MAP_GET_ID = 1;
                int const MAP_PUT_ID = 2;
                int const MAP_REMOVE_ID = 9;

                int const CLIENT_PORTABLE_FACTORY = -3;
                int const HAZELCAST_SERVER_ERROR_ID = 1;
                int const AUTHENTICATION_REQUEST_ID = 2;
                int const PRINCIPAL_ID = 3;

                int const DATA_FACTORY_ID = 0;
                int const CLIENT_PING_REQUEST_ID = 9;

                const byte protocol_bytes[3] = {'C', 'B', '1'};
                std::vector<byte> const PROTOCOL(protocol_bytes, protocol_bytes + 3);
            }

            namespace SpiConstants {
                int const SPI_PORTABLE_FACTORY = -1;
                int const CREDENTIALS_ID = 1;
            }
        }
    }
}

#endif