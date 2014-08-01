//
// Created by sancar koyunlu on 19/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            IdentifiedDataSerializableResponse::~IdentifiedDataSerializableResponse() {

            }

            void IdentifiedDataSerializableResponse::writeData(serialization::ObjectDataOutput &writer) const{
                throw exception::HazelcastSerializationException(" Response::writeData(serialization::ObjectDataOutput &writer)", "write of Client responses is not implemented ");
            }
        }
    }
}

