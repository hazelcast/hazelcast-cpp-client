//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "hazelcast/util/HazelcastDll.h"
#include <boost/thread/future.hpp>

namespace hazelcast {
    namespace util {
        class CallPromise;
    }
    namespace client {
        class Address;

        class ClientConfig;

        namespace serialization {
            class Data;
        }
        namespace impl {
            class PortableRequest;

            class EventHandlerWrapper;
        }

        namespace connection {
            class Connection;
        }
        namespace spi {

            class ClientContext;

            class HAZELCAST_API InvocationService {
            public:
                InvocationService(spi::ClientContext &clientContext);

                boost::shared_future<serialization::Data> invokeOnRandomTarget(const impl::PortableRequest &request);

                boost::shared_future<serialization::Data> invokeOnKeyOwner(const impl::PortableRequest &request, serialization::Data &key);

                boost::shared_future<serialization::Data> invokeOnTarget(const impl::PortableRequest &request, const Address &target);

                boost::shared_future<serialization::Data> invokeOnRandomTarget(const impl::PortableRequest &request, impl::EventHandlerWrapper *handler);

                boost::shared_future<serialization::Data> invokeOnTarget(const impl::PortableRequest &request, impl::EventHandlerWrapper *handler, const Address &target);

                boost::shared_future<serialization::Data> invokeOnKeyOwner(const impl::PortableRequest &request, impl::EventHandlerWrapper *handler, const serialization::Data &key);

                void resend(util::CallPromise *promise);

                bool isRedoOperation() const;

                static const int RETRY_COUNT = 20;
                static const int RETRY_WAIT_TIME = 500;
            private :
                const bool redoOperation;
                spi::ClientContext &clientContext;

                boost::shared_future<serialization::Data> doSend(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, connection::Connection &connection);

                connection::Connection *getOrConnect(const Address *target);

            };
        }
    }
}


#endif //__InvocationService_H_
