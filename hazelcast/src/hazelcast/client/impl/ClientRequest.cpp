//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace impl {


            ClientRequest::ClientRequest():callId(0) {

            }

            void ClientRequest::readPortable(serialization::PortableReader &reader) {
                throw exception::HazelcastSerializationException("  void Request::readPortable(serialization::PortableReader& reader)", "read of Client requests is not implemented ");
            }


            int ClientRequest::getClassVersion() const {
                return 1;
            }

            void ClientRequest::writePortable(serialization::PortableWriter &writer) const {
                writer.writeInt("cId", callId);
                write(writer);
            }

            ClientRequest::~ClientRequest() {

            }

            bool ClientRequest::isRetryable() const {
                return false;
            }

            bool ClientRequest::isBindToSingleConnection() const {
                return false;
            }
        }
    }
}


