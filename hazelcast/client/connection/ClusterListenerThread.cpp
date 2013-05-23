//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "ClusterListenerThread.h"
#include "Connection.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            ClusterListenerThread::ClusterListenerThread() {

            };

            void ClusterListenerThread::setInitialConnection(hazelcast::client::connection::Connection *connection) {
                this->connection = connection;
            };

            void ClusterListenerThread::start() {

            };

        }
    }
}