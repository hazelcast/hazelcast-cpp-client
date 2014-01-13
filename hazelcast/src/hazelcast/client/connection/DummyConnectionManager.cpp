//
// Created by sancar koyunlu on 8/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/connection/DummyConnectionManager.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace connection {

            DummyConnectionManager::DummyConnectionManager(spi::ClientContext& clientContext)
            : ConnectionManager(clientContext)
            , isAddressAvailable(false) {

            }

//            boost::shared_ptr<Connection>DummyConnectionManager::firstConnection(const Address &address) {
//                boost::shared_ptr<Connection>connection = newConnection(address);
//                this->address = connection->getEndpoint();
//                return connection;
//            }
//
//            boost::shared_ptr<Connection>DummyConnectionManager::getConnection(Address const &address) {
//                if (isAddressAvailable)
//                    return ConnectionManager::getConnection(this->address);
//                else
//                    return ConnectionManager::getConnection(address);
//            }


        }
    }
}
