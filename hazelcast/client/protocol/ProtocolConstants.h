#ifndef HAZELCAST_PROTOCOL_CONSTANTS
#define HAZELCAST_PROTOCOL_CONSTANTS


namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace ProtocolConstants {
                int const CLIENT_PORTABLE_FACTORY = -3;
                int const HAZELCAST_SERVER_ERROR_ID = 1;
                int const AUTHENTICATION_REQUEST_ID = 2;
                int const PRINCIPAL_ID = 3;

                int const DATA_FACTORY_ID = 0;
                int const CLIENT_PING_REQUEST_ID = 9;
            }

            namespace SpiConstants {
                int const SPI_PORTABLE_FACTORY = -1;
                int const CREDENTIALS_ID = 1;
            }
        }
    }
}

#endif