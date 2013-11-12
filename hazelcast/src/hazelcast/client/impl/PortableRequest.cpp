//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PortableRequest.h"
#include "IOException.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            void PortableRequest::readPortable(serialization::PortableReader &reader) {
                throw exception::IOException("  void Request::readPortable(serialization::PortableReader& reader)", "read of Client requests is not implemented ");
            }

            PortableRequest::~PortableRequest() {

            }
        }
    }
}

