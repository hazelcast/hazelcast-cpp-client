/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 8/2/13.





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

            class TxnMapRequest : public txn::BaseTxnRequest {
            public:

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, const serialization::pimpl::Data &key);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value);

                TxnMapRequest(const std::string &name, TxnMapRequestType requestType, const serialization::pimpl::Data &key, const serialization::pimpl::Data &value, const serialization::pimpl::Data &newValue);

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

