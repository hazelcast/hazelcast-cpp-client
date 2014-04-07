//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_UTIL_FUNCTIONS
#define HAZELCAST_UTIL_FUNCTIONS

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <map>
#include <vector>
#include <cstring>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            namespace pimpl {
                class Data;
            }
        }
    }
    namespace util {

        HAZELCAST_API long getThreadId();

        HAZELCAST_API void writeNullableData(client::serialization::ObjectDataOutput &out, const client::serialization::pimpl::Data *data);

        HAZELCAST_API void readNullableData(client::serialization::ObjectDataInput &in, client::serialization::pimpl::Data *data);

        HAZELCAST_API long getCurrentTimeMillis();

    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS
