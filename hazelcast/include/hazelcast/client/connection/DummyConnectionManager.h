//
// Created by sancar koyunlu on 8/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_DummyConnectionManager
#define HAZELCAST_DummyConnectionManager

#include "ConnectionManager.h"
#include "Address.h"


namespace hazelcast {
    namespace client {
        class Address;
        namespace connection {

            class DummyConnectionManager : public ConnectionManager {
            public:
                DummyConnectionManager(spi::ClusterService& clusterService, serialization::SerializationService&, ClientConfig&);

                virtual Connection *newConnection(Address const & address);

                virtual Connection *getConnection(Address const & address);

            private:
                Address address;
                bool isAddressAvailable;
            };
        }
    }
}


#endif //HAZELCAST_DummyConnectionManager
