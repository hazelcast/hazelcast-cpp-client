//
//  SerializationConstants.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_SERIALIZATION_CONSTANTS
#define HAZELCAST_SERIALIZATION_CONSTANTS

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <vector>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class HAZELCAST_API SerializationConstants {
                public:
                    static int const CONSTANT_TYPE_NULL;
                    static int const CONSTANT_TYPE_PORTABLE;
                    static int const CONSTANT_TYPE_DATA;
                    static int const CONSTANT_TYPE_BYTE;
                    static int const CONSTANT_TYPE_BOOLEAN;
                    static int const CONSTANT_TYPE_CHAR;
                    static int const CONSTANT_TYPE_SHORT;
                    static int const CONSTANT_TYPE_INTEGER;
                    static int const CONSTANT_TYPE_LONG;
                    static int const CONSTANT_TYPE_FLOAT;
                    static int const CONSTANT_TYPE_DOUBLE;
                    static int const CONSTANT_TYPE_STRING;
                    static int const CONSTANT_TYPE_BYTE_ARRAY;
                    static int const CONSTANT_TYPE_CHAR_ARRAY;
                    static int const CONSTANT_TYPE_SHORT_ARRAY;
                    static int const CONSTANT_TYPE_INTEGER_ARRAY;
                    static int const CONSTANT_TYPE_LONG_ARRAY;
                    static int const CONSTANT_TYPE_FLOAT_ARRAY;
                    static int const CONSTANT_TYPE_DOUBLE_ARRAY;
                    static int const CONSTANT_SERIALIZERS_LENGTH;
                    // ------------------------------------------------------------
                    static int const DEFAULT_TYPE_CLASS;
                    static int const DEFAULT_TYPE_DATE;
                    static int const DEFAULT_TYPE_BIG_INTEGER;
                    static int const DEFAULT_TYPE_BIG_DECIMAL;
                    static int const DEFAULT_TYPE_OBJECT;
                    static int const DEFAULT_TYPE_EXTERNALIZABLE;
                    static int const DEFAULT_TYPE_ENUM;

                    // ------------------------------------------------------------
                    // AUTOMATICALLY REGISTERED SERIALIZERS

                    static int const AUTO_TYPE_ARRAY_LIST;

                    static int const AUTO_TYPE_JOB_PARTITION_STATE;

                    static int const AUTO_TYPE_JOB_PARTITION_STATE_ARRAY;

                    static int const AUTO_TYPE_LINKED_LIST;
                    // ------------------------------------------------------------
                    // HIBERNATE SERIALIZERS

                    static int const HIBERNATE3_TYPE_HIBERNATE_CACHE_KEY;
                    static int const HIBERNATE3_TYPE_HIBERNATE_CACHE_ENTRY;

                    static int const HIBERNATE4_TYPE_HIBERNATE_CACHE_KEY;
                    static int const HIBERNATE4_TYPE_HIBERNATE_CACHE_ENTRY;
/*
                    SerializationConstants();

                    std::string typeIdToName(int typeId);

                private:
                    int size;
                    std::vector<std::string> typeIdNameVector;

                    int idToIndex(int id);*/

                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_SERIALIZATION_CONSTANTS */

