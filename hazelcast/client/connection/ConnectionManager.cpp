//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ConnectionManager.h"
#include "ClientPingRequest.h"
#include "ClientConfig.h"
#include "AuthenticationRequest.h"
#include "HazelcastServerError.h"
#include "SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionManager::ConnectionManager(hazelcast::client::serialization::SerializationService& serializationService, hazelcast::client::ClientConfig& clientConfig)
            : serializationService(serializationService)
            , clientConfig(clientConfig)
            , heartBeatChecker(5, serializationService)
            , live(true) { //TODO get from config

            };

            Connection & ConnectionManager::newConnection(Address const & address) {
                Connection *connection = new Connection(address, serializationService);
                authenticate(*connection, clientConfig.getCredentials(), false);
//                return ;
            };


            void ConnectionManager::authenticate(Connection& connection, const hazelcast::client::protocol::Credentials& credentials, bool reAuth) {
                hazelcast::client::protocol::AuthenticationRequest auth(credentials);
                auth.setReAuth(reAuth);

                connection.write(serializationService.toData(auth));
                hazelcast::client::serialization::Data data;
                connection.read(data);
                if (data.isServerError()) {
                    throw serializationService.toObject<hazelcast::client::protocol::HazelcastServerError>(data);
                } else {
                    principal = serializationService.toObject<hazelcast::client::protocol::Principal>(data);
                }
            };

            void ConnectionManager::checkLive() {
                if (!live) {
                    throw hazelcast::client::HazelcastException("Instance not active!");
                }
            }


        }
    }
}