//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "HeartBeatChecker.h"
#include "Connection.h"
#include "hazelcast/client/../util/Thread.h"
#include "hazelcast/client/serialization/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            struct HeartBeatThreadArgs {
                Connection& connection;
                serialization::Data& ping;
            };

            HeartBeatChecker::HeartBeatChecker(int timeout, serialization::SerializationService& serializationService)
            :serializationService(serializationService)
            , timeout(timeout)
            , ping(serializationService.toData<protocol::PingRequest>(&pingRequest)){
            }

            void *HeartBeatChecker::run(void *input) {
                HeartBeatThreadArgs *args = (HeartBeatThreadArgs *) input;
                try{
                    args->connection.write(args->ping);
                }catch(...){
                    std::cerr << "Warning: HearBeatChecker ping failed " << std::endl;
                }
            };

            bool HeartBeatChecker::checkHeartBeat(Connection& connection) {
                HeartBeatThreadArgs args = {connection, ping};
                if ((clock() - connection.getLastReadTime()) > timeout * CLOCKS_PER_SEC / 2) {
                    util::Thread w(HeartBeatChecker::run, &args);
                    w.start();
                    return w.join(timeout * 1000 * 1000);
                } else {
                    return true;
                }
            }

        }
    }
}