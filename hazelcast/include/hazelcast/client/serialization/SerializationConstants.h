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

namespace hazelcast {
    namespace client {
        namespace serialization {

            class HAZELCAST_API SerializationConstants {
            public:
                static int const CONSTANT_TYPE_PORTABLE = -1;
                static int const CONSTANT_TYPE_DATA = -2;
                static int const CONSTANT_TYPE_BYTE = -3;
                static int const CONSTANT_TYPE_BOOLEAN = -4;
                static int const CONSTANT_TYPE_CHAR = -5;
                static int const CONSTANT_TYPE_SHORT = -6;
                static int const CONSTANT_TYPE_INTEGER = -7;
                static int const CONSTANT_TYPE_LONG = -8;
                static int const CONSTANT_TYPE_FLOAT = -9;
                static int const CONSTANT_TYPE_DOUBLE = -10;
                static int const CONSTANT_TYPE_STRING = -11;
                static int const CONSTANT_TYPE_BYTE_ARRAY = -12;
                static int const CONSTANT_TYPE_CHAR_ARRAY = -13;
                static int const CONSTANT_TYPE_SHORT_ARRAY = -14;
                static int const CONSTANT_TYPE_INTEGER_ARRAY = -15;
                static int const CONSTANT_TYPE_LONG_ARRAY = -16;
                static int const CONSTANT_TYPE_FLOAT_ARRAY = -17;
                static int const CONSTANT_TYPE_DOUBLE_ARRAY = -18;
                static int const CONSTANT_SERIALIZERS_LENGTH = -(-19) - 1;
                // ------------------------------------------------------------
                static int const DEFAULT_TYPE_CLASS = -19;
                static int const DEFAULT_TYPE_DATE = -20;
                static int const DEFAULT_TYPE_BIG_INTEGER = -21;
                static int const DEFAULT_TYPE_BIG_DECIMAL = -22;
                static int const DEFAULT_TYPE_OBJECT = -23;
                static int const DEFAULT_TYPE_EXTERNALIZABLE = -24;
                static int const DEFAULT_TYPE_ENUM = -25;

                SerializationConstants();

                std::string typeIdToName(int typeId);

            private:
                int size;
                std::vector<std::string> typeIdNameVector;

                int idToIndex(int id);

            };

        }
    }
}
#endif /* HAZELCAST_SERIALIZATION_CONSTANTS */
