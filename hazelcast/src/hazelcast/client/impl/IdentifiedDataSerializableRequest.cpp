//
// Created by sancar koyunlu on 12/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "IdentifiedDataSerializableRequest.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            IdentifiedDataSerializableRequest::~IdentifiedDataSerializableRequest() {

            }

            void IdentifiedDataSerializableRequest::readData(serialization::ObjectDataInput &reader) {
                throw exception::IOException("  void Request::readData(serialization::ObjectDataInput &reader", "read of Client requests is not implemented ");
            }
        }
    }
}
