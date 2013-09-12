//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "Request.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            void Request::readPortable(serialization::PortableReader& reader) {
                throw exception::IOException("  void Request::readPortable(serialization::PortableReader& reader)", "read of Client requests is not implemented ");
            }

            Request::~Request() {

            }


        }
    }
}

