//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "HeartBeatChecker.h"
#include "Connection.h"
#include "../protocol/ClientPingRequest.h"
#include "../serialization/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            HeartBeatChecker::HeartBeatChecker(int timeout, hazelcast::client::serialization::SerializationService& serializationService)
            :serializationService(serializationService)
            , timeout(timeout) {
                hazelcast::client::protocol::ClientPingRequest clientPingRequest;
                serializationService.toData(clientPingRequest, ping);
            }


            bool HeartBeatChecker::checkHeartBeat(Connection const & connection) {
                return true;
//            if ((Clock.currentTimeMillis() - connection.getLastReadTime()) > connectionTimeout / 2) {
//                final CountDownLatch latch = new CountDownLatch(1);
//                executionService.execute(new Runnable() {
//                    public void run() {
//                        try {
//                            connection.write(ping);
//                            latch.countDown();
//                        } catch (IOException e) {
//                            e.printStackTrace();
//                        }
//                    }
//                });
//                try {
//                    return latch.await(5, TimeUnit.SECONDS);
//                } catch (InterruptedException e) {
//                    return false;
//                }
//            } else {
//                return true;
//            }
            }

        }
    }
}