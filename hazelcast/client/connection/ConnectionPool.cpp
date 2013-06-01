//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "ConnectionPool.h"
#include "../Address.h"
#include "Connection.h"
#include "ConnectionManager.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ConnectionPool::ConnectionPool(const Address& address, hazelcast::client::serialization::SerializationService& serializationService)
            : address(address)
            , active(true)
            , serializationService(serializationService) {

            };

            ConnectionPool::~ConnectionPool() {
                destroy();
            };

            Connection *ConnectionPool::take(ConnectionManager *manager) {
                if (!active) {
                    return NULL;
                }
                Connection *t;
                bool b = queue.poll(t);
                if (b == false) {
                    t = new Connection(address, serializationService);
                    manager->authenticate(*t, false);
                }
                return t;
            }

            void ConnectionPool::release(Connection *e) {
                if (!active || !queue.offer(e)) {
                    e->close();
                    delete e;
                }
            }

            void ConnectionPool::destroy() {
                active = false;
                while (queue.empty()) {
                    Connection *connection;
                    queue.poll(connection);
                    delete connection;
                }
            }
        }
    }
}