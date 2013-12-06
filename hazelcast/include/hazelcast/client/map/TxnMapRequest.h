//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMapRequest
#define HAZELCAST_TxnMapRequest

#include "hazelcast/client/impl/PortableRequest.h"
#include <memory>
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {

            class HAZELCAST_API TxnMapRequestType {
            public:
                enum Type {
                    NONE = 0,
                    CONTAINS_KEY = 1,
                    GET = 2,
                    SIZE = 3,
                    PUT = 4,
                    PUT_IF_ABSENT = 5,
                    REPLACE = 6,
                    REPLACE_IF_SAME = 7,
                    SET = 8,
                    REMOVE = 9,
                    DELETE_R = 10,
                    REMOVE_IF_SAME = 11,
                    KEYSET = 12,
                    KEYSET_BY_PREDICATE = 13,
                    VALUES = 14,
                    VALUES_BY_PREDICATE = 15

                } value;

                TxnMapRequestType();

                TxnMapRequestType(Type value);

                operator int() const;

                void operator = (int i);

            private:
                std::vector<Type> types;
            };

            class HAZELCAST_API TxnMapRequest : public impl::PortableRequest {
            public:
                TxnMapRequest();

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, serialization::Data *key);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value, serialization::Data *newValue);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, const std::string &predicate);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                TxnMapRequestType requestType;
                serialization::Data *key;
                serialization::Data *value;
                serialization::Data *newValue;
                const std::string *predicate;
            };

        }
    }
}

#endif //HAZELCAST_TxnMapRequest
