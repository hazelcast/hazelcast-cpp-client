//
// Created by sancar koyunlu on 8/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnMapRequest.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {

            int TxnMapRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int TxnMapRequest::getClassId() const {
                return PortableHook::TXN_REQUEST;
            };


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


        }
    }
}

