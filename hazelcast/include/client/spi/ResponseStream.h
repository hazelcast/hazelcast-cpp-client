//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.





#ifndef HAZELCAST_RESPONSE_STREAM
#define HAZELCAST_RESPONSE_STREAM

#include "../serialization/SerializationService.h"
#include "../serialization/Data.h"
#include "../connection/Connection.h"
#include "../../util/Lock.h"
#include "../../util/LockGuard.h"

namespace hazelcast {
    namespace client {

        namespace serialization {
            class SerializationService;
        }

        namespace connection {
            class Connection;
        }
        namespace spi {


            class ResponseStream {
            public:
                ResponseStream(serialization::SerializationService& serializationService, connection::Connection& connection);

                template<typename T>
                T read() {
                    serialization::Data data = connection.read(serializationService.getSerializationContext());
                    return serializationService.toObject<T>(data);
                };

                void end() {
                    util::LockGuard lg(endMutex);
                    if (!isEnded) {
                        connection.close();
                        isEnded = true;
                    }
                };

            private:
                serialization::SerializationService& serializationService;
                connection::Connection& connection;
                volatile bool isEnded;
                util::Lock endMutex;
            };
        }
    }
}

#endif //HAZELCAST_RESPONSE_STREAM
