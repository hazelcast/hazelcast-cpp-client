////
//// Created by sancar koyunlu on 5/21/13.
//// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
//
//#include "hazelcast/client/connection/HeartBeatChecker.h"
//#include "hazelcast/client/connection/Connection.h"
//#include "hazelcast/client/serialization/pimpl/SerializationService.h"
//#include "hazelcast/util/Thread.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace connection {
//
//
//            HeartBeatChecker::HeartBeatChecker(int timeout, serialization::pimpl::SerializationService& serializationService)
//            :serializationService(serializationService)
//            , timeout(timeout)
//            , ping(serializationService.toData<protocol::PingRequest>(&pingRequest)) {
//            }
//
//            void HeartBeatChecker::run(boost::shared_ptr<Connection>connection) {
//                try{
//                    connection->write(ping);
//                    connection->read();
//                }catch(...){
//                    logger "Warning: HearBeatChecker ping failed " << std::endl;
//                    sleep(5)
//                }
//            };
//
//            bool HeartBeatChecker::checkHeartBeat(Connection& connection) {
//                if ((clock() - connection.getLastReadTime()) > timeout * ((CLOCKS_PER_SEC / 1000) / 2)) {
//                    util::Thread thread(&HeartBeatChecker::run, this, &connection));
//                    return thread.try_join_for(timeout);
//                } else {
//                    return true;
//                }
//            }
//
//        }
//    }
//}
