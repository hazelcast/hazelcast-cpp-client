//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMapRequest
#define HAZELCAST_TxnMapRequest

#include "Portable.h"
#include <memory>
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace map {

            class TxnMapRequestType {
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
                    DELETE = 10,
                    REMOVE_IF_SAME = 11

                } value;

                TxnMapRequestType();

                TxnMapRequestType(Type value);

                operator int() const;

                void operator = (int i);

            private:
                std::vector<Type> types;
            };

            class TxnMapRequest : public Portable {
            public:
                TxnMapRequest();

                TxnMapRequest(const std::string& name, TxnMapRequestType requestType);

                TxnMapRequest(const std::string& name, TxnMapRequestType requestType, serialization::Data *key);

                TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value);

                TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value, serialization::Data *newValue);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                std::string name;
                TxnMapRequestType requestType;
                std::auto_ptr<serialization::Data> key;
                std::auto_ptr<serialization::Data> value;
                std::auto_ptr<serialization::Data> newValue;
            };

        }
    }
}

#endif //HAZELCAST_TxnMapRequest
