//
// Created by sancar koyunlu on 8/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "DummyConnectionManager.h"
#include "Connection.h"
#include "ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            DummyConnectionManager::DummyConnectionManager(spi::ClusterService& clusterService, serialization::SerializationService & service, ClientConfig & config)
            : ConnectionManager(clusterService, service, config)
            , isAddressAvailable(false) {

            }

            Connection *DummyConnectionManager::newConnection(Address const & address) {
                Connection *connection = ConnectionManager::newConnection(address);
                this->address = connection->getEndpoint();
                isAddressAvailable = true;
                return connection;
            }

            Connection *DummyConnectionManager::getConnection(Address const & address) {
                if (isAddressAvailable)
                    return ConnectionManager::getConnection(this->address);
                else
                    return ConnectionManager::getConnection(address);
            }


        }
    }
}
