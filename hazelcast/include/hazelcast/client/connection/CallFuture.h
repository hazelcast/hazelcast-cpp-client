//
// Created by sancar koyunlu on 21/08/14.
//


#ifndef HAZELCAST_CallFuture
#define HAZELCAST_CallFuture

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace spi{
            class InvocationService;
        }

        namespace connection {
            class CallPromise;

            class Connection;

            class CallFuture {
            public:
                CallFuture();

                CallFuture(boost::shared_ptr<CallPromise> promise, boost::shared_ptr<Connection> connection, int heartBeatTimeout, spi::InvocationService* invocationService);

                // copy constructor
                CallFuture(const CallFuture &rhs);

                CallFuture &operator =(const CallFuture &rhs);

                serialization::pimpl::Data get();

                serialization::pimpl::Data get(time_t timeoutInSeconds);

            private:
                boost::shared_ptr<CallPromise> promise;
                boost::shared_ptr<Connection> connection;
                spi::InvocationService* invocationService;
                int heartBeatTimeout;
            };

        }
    }
}

#endif //HAZELCAST_CallFuture
