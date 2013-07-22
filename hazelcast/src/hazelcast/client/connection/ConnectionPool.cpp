//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ConnectionPool.h"
#include "Connection.h"
#include "ConnectionManager.h"


namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionPool::ConnectionPool(const Address& address, hazelcast::client::serialization::SerializationService& serializationService, ConnectionManager &connectionManager)
            : address(address)
            , active(true)
            , serializationService(serializationService)
            , connectionManager(connectionManager) {

            };

            ConnectionPool::~ConnectionPool() {
                active = false;
                while (!queue.empty()) {
                    Connection *connection;
                    if (queue.poll(connection)) {
                        delete connection;
                    }
                }
            };

            Connection *ConnectionPool::take() {
                if (!active) {
                    return NULL;
                }
                Connection *t;
                bool b = queue.poll(t);
                if (b == false) {
                    t = new Connection(address, serializationService);
                    connectionManager.authenticate(*t, false, false);
                }
                return t;
            }

            void ConnectionPool::release(Connection *e) {
                if (!active || !queue.offer(e)) {
                    delete e;
                }
            }
        }
    }
}