//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "GetRequest.h"

namespace hazelcast {
    namespace client {
        namespace map {
            GetRequest::GetRequest(const std::string& name, hazelcast::client::serialization::Data& key)
            :name(name)
            , key(key) {

            };


        }
    }
}