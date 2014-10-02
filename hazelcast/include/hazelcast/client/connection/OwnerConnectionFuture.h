//
// Created by sancar koyunlu on 19/08/14.
//


#ifndef HAZELCAST_OwnerConnectionFuture
#define HAZELCAST_OwnerConnectionFuture

#include "hazelcast/util/HazelcastDll.h"
#include <boost/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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

                void close();

            private:
                boost::shared_ptr<Connection> ownerConnectionPtr;
                spi::ClientContext& clientContext;
            };
        }
    }
}
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_OwnerConnectionFuture
