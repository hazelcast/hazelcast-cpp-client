//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMapRequest.h"
#include "PortableHook.h"
#include "Data.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            TxnMapRequestType::TxnMapRequestType()
            :value(NONE) {
                types.resize(11);
                types[0] = NONE;
                types[1] = CONTAINS_KEY;
                types[2] = GET;
                types[3] = SIZE;
                types[4] = PUT;
                types[5] = PUT_IF_ABSENT;
                types[6] = REPLACE;
                types[7] = REPLACE_IF_SAME;
                types[8] = SET;
                types[9] = REMOVE;
                types[10] = DELETE;
                types[11] = REMOVE_IF_SAME;
            };

            TxnMapRequestType::TxnMapRequestType(TxnMapRequestType::Type value)
            :value(value) {
                types.resize(11);
                types[0] = NONE;
                types[1] = CONTAINS_KEY;
                types[2] = GET;
                types[3] = SIZE;
                types[4] = PUT;
                types[5] = PUT_IF_ABSENT;
                types[6] = REPLACE;
                types[7] = REPLACE_IF_SAME;
                types[8] = SET;
                types[9] = REMOVE;
                types[10] = DELETE;
                types[11] = REMOVE_IF_SAME;
            };

            TxnMapRequestType::operator int() const {
                return value;
            };

            void TxnMapRequestType::operator = (int i) {
                value = types[i];
            };

            //----------------------------------//
            TxnMapRequest::TxnMapRequest()
            : key(NULL)
            , value(NULL)
            , newValue(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType)
            : name(name)
            , requestType(requestType)
            , key(NULL)
            , value(NULL)
            , newValue(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType, serialization::Data *key)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(NULL)
            , newValue(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(value)
            , newValue(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value, serialization::Data *newValue)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(value)
            , newValue(newValue) {
            };

            int TxnMapRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int TxnMapRequest::getClassId() const {
                return PortableHook::TXN_REQUEST;
            };


            void TxnMapRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                util::writeNullableData(out, key.get());
                util::writeNullableData(out, value.get());
                util::writeNullableData(out, newValue.get());
            };


            void TxnMapRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key->readData(in);
                value->readData(in);
                newValue->readData(in);
            };

        }
    }
}

