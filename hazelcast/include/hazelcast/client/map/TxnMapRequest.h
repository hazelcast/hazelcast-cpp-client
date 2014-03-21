//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_TxnMapRequest
#define HAZELCAST_TxnMapRequest

#include "hazelcast/client/txn/BaseTxnRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <memory>
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class Data;
            }

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

                TxnMapRequestType(Type value);

                operator int() const;

                void operator = (int i);

            private:
                std::vector<Type> types;
            };

            class HAZELCAST_API TxnMapRequest : public txn::BaseTxnRequest {
            public:

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, serialization::pimpl::Data &key);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, serialization::pimpl::Data &key, serialization::pimpl::Data &value);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, serialization::pimpl::Data &key, serialization::pimpl::Data &value, serialization::pimpl::Data &newValue);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, const std::string &predicate);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                std::string name;
                TxnMapRequestType requestType;
                serialization::pimpl::Data key;
                serialization::pimpl::Data value;
                serialization::pimpl::Data newValue;
                bool hasKey;
                bool hasValue;
                bool hasNewValue;
                bool hasPredicate;
                std::string predicate;
                long ttl;
            };

        }
    }
}

#endif //HAZELCAST_TxnMapRequest
