//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace impl {


            PortableRequest::PortableRequest():callId(0) {

            }

            void PortableRequest::readPortable(serialization::PortableReader &reader) {
                throw exception::IOException("  void Request::readPortable(serialization::PortableReader& reader)", "read of Client requests is not implemented ");
            }


            void PortableRequest::writePortable(serialization::PortableWriter &writer) const {
                writer.writeInt("cId", callId);
                write(writer);
            }

            PortableRequest::~PortableRequest() {

            }

            bool PortableRequest::isRetryable() const {
                return false;
            }

            bool PortableRequest::isBindToSingleConnection() const {
                return false;
            }
        }
    }
}


