//
// Created by sancar koyunlu on 19/08/14.
//


#ifndef HAZELCAST_OwnerConnectionFuture
#define HAZELCAST_OwnerConnectionFuture

#include <boost/shared_ptr.hpp>

namespace hazelcast {
    namespace client {
        class Address;

        namespace spi {
            class ClientContext;
        }

        namespace connection {
            class Connection;

            class HAZELCAST_API OwnerConnectionFuture {
            public:
                OwnerConnectionFuture(spi::ClientContext&);

                void markAsClosed();

                boost::shared_ptr<Connection> getOrWaitForCreation();

                boost::shared_ptr<Connection> createNew(const Address& address);

                void closeIfAddressMatches(const Address& address);

            private:
                boost::shared_ptr<Connection> ownerConnectionPtr;
                spi::ClientContext& clientContext;
            };
        }
    }
}


#endif //HAZELCAST_OwnerConnectionFuture
