//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ResponseStream.h"

namespace hazelcast {
    namespace client {

        namespace spi {

            ResponseStream::ResponseStream(serialization::SerializationService& serializationService, connection::Connection& connection)
            :serializationService(serializationService)
            , connection(connection) {

            };

        }
    }
}