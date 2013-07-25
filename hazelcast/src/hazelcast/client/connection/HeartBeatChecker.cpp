//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "HeartBeatChecker.h"
#include "Connection.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include <boost/thread.hpp>

namespace hazelcast {
    namespace client {
        namespace connection {


            HeartBeatChecker::HeartBeatChecker(int timeout, serialization::SerializationService& serializationService)
            :serializationService(serializationService)
            , timeout(timeout)
            , ping(serializationService.toData<protocol::PingRequest>(&pingRequest)) {
            }

            void HeartBeatChecker::run(Connection *connection) {
                try{
                    connection->write(ping);
                    connection->read(serializationService.getSerializationContext());
                }catch(...){
                    std::cerr << "Warning: HearBeatChecker ping failed " << std::endl;
                    boost::this_thread::sleep(boost::posix_time::seconds(5));
                }
            };

            bool HeartBeatChecker::checkHeartBeat(Connection& connection) {
                if ((clock() - connection.getLastReadTime()) > timeout * ((CLOCKS_PER_SEC / 1000) / 2)) {
                    boost::thread thread(boost::bind(&HeartBeatChecker::run, this, &connection));
                    return thread.try_join_for(boost::chrono::duration<int, boost::milli>(timeout));
                } else {
                    return true;
                }
            }

        }
    }
}