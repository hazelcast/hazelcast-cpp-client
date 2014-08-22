//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "hazelcast/util/HazelcastDll.h"
#include "boost/shared_ptr.hpp"

namespace hazelcast {

    namespace client {
        class Address;

        class ClientConfig;

        namespace serialization {
            namespace pimpl {
                class Data;
            }

        }
        namespace impl {
            class ClientRequest;

            class BaseEventHandler;
        }

        namespace connection {
            class Connection;

            class CallFuture;
        }
        namespace spi {

            class ClientContext;

            class HAZELCAST_API InvocationService {
            public:
                InvocationService(spi::ClientContext& clientContext);

                void start();

                connection::CallFuture invokeOnRandomTarget(const impl::ClientRequest *request);

                connection::CallFuture invokeOnKeyOwner(const impl::ClientRequest *request, int partitionId);

                connection::CallFuture invokeOnTarget(const impl::ClientRequest *request, const Address& target);

                connection::CallFuture invokeOnRandomTarget(const impl::ClientRequest *request, impl::BaseEventHandler *handler);

                connection::CallFuture invokeOnTarget(const impl::ClientRequest *request, impl::BaseEventHandler *handler, const Address& target);

                connection::CallFuture invokeOnKeyOwner(const impl::ClientRequest *request, impl::BaseEventHandler *handler, int partitionId);

                connection::CallFuture invokeOnConnection(const impl::ClientRequest *request, boost::shared_ptr<connection::Connection> connection);

                bool isRedoOperation() const;

                int getRetryWaitTime() const;

                int getRetryCount() const;

                int getMaxFailedHeartbeatCount() const;

            private:
                bool redoOperation;
                int heartbeatTimeout;
                int maxFailedHeartbeatCount;
                int retryWaitTime;
                int retryCount;
                spi::ClientContext& clientContext;

                connection::CallFuture doSend(std::auto_ptr<const impl::ClientRequest>, std::auto_ptr<impl::BaseEventHandler>, boost::shared_ptr<connection::Connection>, int);


            };
        }
    }
}


#endif //__InvocationService_H_

