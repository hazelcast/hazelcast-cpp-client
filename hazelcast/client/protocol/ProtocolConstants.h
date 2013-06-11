#ifndef HAZELCAST_PROTOCOL_CONSTANTS
#define HAZELCAST_PROTOCOL_CONSTANTS

#include <vector>

namespace hazelcast {
    namespace client {
        typedef unsigned char byte;
        namespace protocol {
            namespace ProtocolConstants {

                int const CLIENT_PORTABLE_FACTORY = -3;
                int const HAZELCAST_SERVER_ERROR_ID = 1;
                int const AUTHENTICATION_REQUEST_ID = 2;
                int const PRINCIPAL_ID = 3;

                int const PARTITION_DS_FACTORY = -2;
                int const GET_PARTITIONS = 1;
                int const PARTITIONS = 2;
                int const ADD_LISTENER = 3;

                int const DATA_FACTORY_ID = 0;
                int const ADDRESS_ID = 1;
                int const MEMBER_ID = 2;
                int const ADD_MS_LISTENER = 7;
                int const MEMBERSHIP_EVENT = 8;
                int const PING = 9;

                const byte protocol_bytes[3] = {'C', 'B', '1'};
                std::vector<byte> const PROTOCOL(protocol_bytes, protocol_bytes + 3);
            }

            namespace SpiConstants {
                int const SPI_PORTABLE_FACTORY = -1;
                int const CREDENTIALS_ID = 1;
                int const COLLECTION = 7;
            }
        }
    }
}

#endif