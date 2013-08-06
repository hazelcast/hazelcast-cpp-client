//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMapRequest
#define HAZELCAST_TxnMapRequest

#include <memory>
#include <bits/stl_vector.h>
#include "Portable.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
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
                TxnMapRequest()
                : key(NULL)
                , value(NULL)
                , newValue(NULL) {
                };

                TxnMapRequest(const std::string& name, TxnMapRequestType requestType)
                : name(name)
                , requestType(requestType)
                , key(NULL)
                , value(NULL)
                , newValue(NULL) {
                };

                TxnMapRequest(const std::string& name, TxnMapRequestType requestType, serialization::Data *key)
                : name(name)
                , requestType(requestType)
                , key(key)
                , value(NULL)
                , newValue(NULL) {
                };

                TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value)
                : name(name)
                , requestType(requestType)
                , key(key)
                , value(value)
                , newValue(NULL) {
                };

                TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value, serialization::Data *newValue)
                : name(name)
                , requestType(requestType)
                , key(key)
                , value(value)
                , newValue(newValue) {
                };

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer.writeUTF("n", name);
                    serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                    util::writeNullableData(out, key.get());
                    util::writeNullableData(out, value.get());
                    util::writeNullableData(out, newValue.get());
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    name = reader.readUTF("n");
                    serialization::ObjectDataInput *in = reader.getRawDataInput();
                    key->readData(*in);
                    value->readData(*in);
                    newValue->readData(*in);
                };

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
