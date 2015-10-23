/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
                int const REMOVE_ALL_LISTENERS = 14;

                int const PARTITION_DS_FACTORY = -2;
                int const PARTITIONS = 2;
                int const ADD_LISTENER = 3;

                int const DATA_FACTORY_ID = 0;
                int const ADDRESS_ID = 1;
                int const MEMBER_ID = 2;
                int const ADD_MS_LISTENER = 7;
                int const MEMBERSHIP_EVENT = 8;
                int const PING = 9;

            }

            namespace SpiConstants {
                int const SPI_PORTABLE_FACTORY = -1;
                int const USERNAME_PWD_CRED = 1;
                int const COLLECTION = 2;
                int const ITEM_EVENT = 3;
                int const ENTRY_EVENT = 4;

                int const SPI_DS_FACTORY = -1;
                int const RESPONSE = 0;
                int const BACKUP = 1;
                int const BACKUP_RESPONSE = 2;
                int const PARTITION_ITERATOR = 3;
                int const PARTITION_RESPONSE = 4;
                int const PARALLEL_OPERATION_FACTORY = 5;
                int const EVENT_PACKET = 6;
                int const DS_COLLECTION = 7;
            }
        }
    }
}

#endif
