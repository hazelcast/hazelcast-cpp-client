//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/TxnMapRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {

            TxnMapRequestType::TxnMapRequestType(TxnMapRequestType::Type value)
            : value(value) {
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
            }

            TxnMapRequestType::operator int() const {
                return value;
            }

            void TxnMapRequestType::operator=(int i) {
                value = types[i];
            }

            //----------------------------------//

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType)
            : name(name)
            , requestType(requestType)
            , hasKey(false)
            , hasValue(false)
            , hasNewValue(false)
            , hasPredicate(false)
            , ttl(-1) {
            }

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType, const serialization::pimpl::Data& key)
            : name(name)
            , requestType(requestType)
            , key(key)
            , hasKey(true)
            , hasValue(false)
            , hasNewValue(false)
            , hasPredicate(false)
            , ttl(-1) {
            }

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(value)
            , hasKey(true)
            , hasValue(true)
            , hasNewValue(false)
            , hasPredicate(false)
            , ttl(-1) {
            }

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType, const serialization::pimpl::Data& key, const serialization::pimpl::Data& value, const serialization::pimpl::Data& newValue)
            : name(name)
            , requestType(requestType)
            , key(key)
            , value(value)
            , newValue(newValue)
            , hasKey(true)
            , hasValue(true)
            , hasNewValue(true)
            , hasPredicate(false)
            , ttl(-1) {
            }

            TxnMapRequest::TxnMapRequest(const std::string& name, TxnMapRequestType requestType, const std::string& predicate)
            : name(name)
            , requestType(requestType)
            , hasKey(false)
            , hasValue(false)
            , hasNewValue(false)
            , hasPredicate(true)
            , predicate(predicate)
            , ttl(-1) {
            }

            int TxnMapRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int TxnMapRequest::getClassId() const {
                return PortableHook::TXN_REQUEST_WITH_SQL_QUERY;
            }

            void TxnMapRequest::write(serialization::PortableWriter& writer) const {
                BaseTxnRequest::write(writer);
                writer.writeUTF("n", name);
                writer.writeInt("t", (int)requestType);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                if (hasKey) {
                    out.writeData(&key);
                } else {
                    out.writeData(NULL);
                }
                if (hasValue) {
                    out.writeData(&value);
                } else {
                    out.writeData(NULL);
                }
                if (hasNewValue) {
                    out.writeData(&newValue);
                } else {
                    out.writeData(NULL);
                }
                out.writeBoolean(hasPredicate);
                if (hasPredicate) {
                    out.writeUTF(predicate);
                }
                out.writeLong(ttl);
            }
        }
    }
}


