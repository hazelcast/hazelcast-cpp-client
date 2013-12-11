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


            class HAZELCAST_API ResponseStream {
            public:
                ResponseStream(serialization::SerializationService &serializationService, connection::Connection *connection);

                template<typename T>
                boost::shared_ptr<T> read() {
                    serialization::Data data = connection->read();
                    return serializationService.toObject<T>(data);
                };

                void end() {
                    boost::lock_guard<boost::mutex> log(endMutex);
                    bool expected = true;
                    if (!isEnded.compare_exchange_strong(expected, false)) {
                        delete connection;
                        isEnded = true;
                    }
                };

            private:
                serialization::SerializationService &serializationService;
                connection::Connection *connection;
                boost::atomic<bool> isEnded;
                boost::mutex endMutex;
            };
        }
    }
}

#endif //HAZELCAST_RESPONSE_STREAM
