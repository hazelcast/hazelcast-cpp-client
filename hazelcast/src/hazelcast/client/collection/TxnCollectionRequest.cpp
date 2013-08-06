//
// Created by sancar koyunlu on 8/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "TxnCollectionRequest.h"


namespace hazelcast {
    namespace client {
        namespace collection {
            TxnCollectionRequest::TxnCollectionRequest(const std::string& name)
            :name(name), data(NULL) {

            };

            TxnCollectionRequest::TxnCollectionRequest(const std::string& name, serialization::Data *data)
            :name(name) {

            };

        }
    }
}