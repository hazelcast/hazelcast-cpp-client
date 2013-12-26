//
// Created by sancar koyunlu on 8/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/DummyConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/spi/ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            DummyConnectionManager::DummyConnectionManager(spi::ClusterService &clusterService, serialization::SerializationService &service, ClientConfig &config)
            : ConnectionManager(clusterService, service, config)
            , isAddressAvailable(false) {

            }

//            Connection *DummyConnectionManager::firstConnection(const Address &address) {
//                Connection *connection = newConnection(address);
//                this->address = connection->getEndpoint();
//                return connection;
//            }
//
//            Connection *DummyConnectionManager::getConnection(Address const &address) {
//                if (isAddressAvailable)
//                    return ConnectionManager::getConnection(this->address);
//                else
//                    return ConnectionManager::getConnection(address);
//            }


        }
    }
}
