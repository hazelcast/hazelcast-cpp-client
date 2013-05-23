//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PutRequest.h"

namespace hazelcast {
    namespace client {
        namespace map {
            PutRequest::PutRequest() {

            };

            PutRequest::PutRequest(const std::string& name, const hazelcast::client::serialization::Data& key, const hazelcast::client::serialization::Data& value, int threadId, long ttl)
            :name(name)
            , key(key)
            , value(value)
            , threadId(threadId)
            , ttl(ttl) {

            };


        }
    }
}