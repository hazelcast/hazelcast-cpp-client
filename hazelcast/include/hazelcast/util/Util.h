//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_UTIL_FUNCTIONS
#define HAZELCAST_UTIL_FUNCTIONS

#include <string>
#include <map>
#include <vector>
#include "Conversion.h"
#include "RetryableRequest.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataOutput;

            class ObjectDataInput;

            class Data;
        }
    }
    namespace util {

        struct cStrCmp {
            bool operator ()(const char *lhs, const char *rhs) const {
                return strcmp(lhs, rhs) < 0;
            }
        };

        std::string to_string(int);

        int getThreadId();

        void writeNullableData(client::serialization::ObjectDataOutput & out, const client::serialization::Data *data);

        void readNullableData(client::serialization::ObjectDataInput & in, client::serialization::Data *data);

        long getCurrentTimeMillis();


        template<typename Request>
        bool isRetryable(Request& request) {
            return util::Conversion<Request, client::RetryableRequest>::exists;
        };

    }
}


#endif //HAZELCAST_UTIL_FUNCTIONS
