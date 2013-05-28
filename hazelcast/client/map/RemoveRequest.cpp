//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "RemoveRequest.h"

namespace hazelcast {
    namespace client {
        namespace map {

            RemoveRequest::RemoveRequest(const std::string& name, hazelcast::client::serialization::Data& key, int threadId)
            :name(name)
            , key(key)
            , threadId(threadId) {

            };


        }
    }
}