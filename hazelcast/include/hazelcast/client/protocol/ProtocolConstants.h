#ifndef HAZELCAST_PROTOCOL_CONSTANTS
#define HAZELCAST_PROTOCOL_CONSTANTS

#include "hazelcast/util/HazelcastDll.h"
#include <vector>

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace ProtocolConstants {
                int const CLIENT_DS_FACTORY = -3;
                int const CLIENT_RESPONSE = 1;

                int const CLIENT_PORTABLE_FACTORY = -3;
                int const HAZELCAST_SERVER_ERROR_ID = 1;
                int const AUTHENTICATION_REQUEST_ID = 2;
                int const PRINCIPAL_ID = 3;
                int const GET_DISTRIBUTED_OBJECT_INFO = 4;
                int const DISTRIBUTED_OBJECT_INFO = 6;
                int const CREATE_PROXY = 7;
                int const DESTROY_PROXY = 8;
                int const LISTENER = 9;
                int const MEMBERSHIP_LISTENER = 10;
                int const CLIENT_PING = 11;
                int const GET_PARTITIONS = 12;
                int const REMOVE_LISTENER = 13;

                int const PARTITION_DS_FACTORY = -2;
                int const PARTITIONS = 2;
                int const ADD_LISTENER = 3;

                int const DATA_FACTORY_ID = 0;
                int const ADDRESS_ID = 1;
                int const MEMBER_ID = 2;
                int const ADD_MS_LISTENER = 7;
                int const MEMBERSHIP_EVENT = 8;
                int const PING = 9;

                const byte protocol_bytes[6] = {'C', 'B', '1', 'C', 'P', 'P'};
                std::vector<byte> const PROTOCOL(protocol_bytes, protocol_bytes + 6);
            }

            namespace SpiConstants {
                int const SPI_PORTABLE_FACTORY = -1;
                int const USERNAME_PWD_CRED = 1;
                int const COLLECTION = 2;
                int const ITEM_EVENT = 3;
                int const ENTRY_EVENT = 4;
            }
        }
    }
}

#endif