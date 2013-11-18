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
                types.resize(16);
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
                types[10] = DELETE_R;
                types[11] = REMOVE_IF_SAME;
                types[12] = KEYSET;
                types[13] = KEYSET_BY_PREDICATE;
                types[14] = VALUES;
                types[15] = VALUES_BY_PREDICATE;
            };

            TxnMapRequestType::TxnMapRequestType(TxnMapRequestType::Type value)
            :value(value) {
                types.resize(16);
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
                types[10] = DELETE_R;
                types[11] = REMOVE_IF_SAME;
                types[12] = KEYSET;
                types[13] = KEYSET_BY_PREDICATE;
                types[14] = VALUES;
                types[15] = VALUES_BY_PREDICATE;
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
            , newValue(NULL)
            , predicate(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType)
            : name(name)
            , requestType(requestType)
            , key(NULL)
            , value(NULL)
            , newValue(NULL)
            , predicate(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType, serialization::Data *key)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(NULL)
            , newValue(NULL)
            , predicate(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(value)
            , newValue(NULL)
            , predicate(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, serialization::Data *key, serialization::Data *value, serialization::Data *newValue)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(value)
            , newValue(newValue)
            , predicate(NULL) {
            };

            TxnMapRequest::TxnMapRequest(const std::string&  name, TxnMapRequestType requestType, const std::string& predicate)
            : name(name)
            , requestType(requestType)
            , key(NULL)
            , value(NULL)
            , newValue(NULL)
            , predicate(&predicate) {
            };

            int TxnMapRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int TxnMapRequest::getClassId() const {
                return PortableHook::TXN_REQUEST_WITH_SQL_QUERY;
            };


            void TxnMapRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("t", (int) requestType);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                util::writeNullableData(out, key);
                util::writeNullableData(out, value);
                util::writeNullableData(out, newValue);
                if (predicate != NULL) {
                    out.writeBoolean(true);
                    out.writeUTF(*predicate);
                } else {
                    out.writeBoolean(false);
                }
            };


        }
    }
}

