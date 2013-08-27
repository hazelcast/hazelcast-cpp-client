//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.





#ifndef HAZELCAST_RESPONSE_STREAM
#define HAZELCAST_RESPONSE_STREAM

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace hazelcast {
    namespace client {

        namespace connection {
            class Connection;
        }
        namespace spi {


            class ResponseStream {
            public:
                ResponseStream(serialization::SerializationService& serializationService, connection::Connection& connection);

                template<typename T>
                T read() {
                    serialization::Data data = connection.read();
                    return serializationService.toObject<T>(data);
                };

                void end() {
                    boost::lock_guard<boost::mutex> log(endMutex);
                    if (!isEnded) {
                        isEnded = true;
                    }
                };

            private:
                serialization::SerializationService& serializationService;
                connection::Connection& connection;
                volatile bool isEnded;
                boost::mutex endMutex;
            };
        }
    }
}

#endif //HAZELCAST_RESPONSE_STREAM
