//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_UTIL_FUNCTIONS
#define HAZELCAST_UTIL_FUNCTIONS

#include "hazelcast/util/Conversion.h"
#include "hazelcast/client/impl/RetryableRequest.h"
#include <string>
#include <map>
#include <vector>
#include <cstring>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            class Data;
        }
    }
    namespace util {

        std::string to_string(int);

        int getThreadId();

        void writeNullableData(client::serialization::ObjectDataOutput &out, const client::serialization::Data *data);

        void readNullableData(client::serialization::ObjectDataInput &in, client::serialization::Data *data);

        long getCurrentTimeMillis();


        template<typename Request>
        bool isRetryable(Request &request) {
            return util::Conversion<Request, client::RetryableRequest>::exists;
        };

    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS
