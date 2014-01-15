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

            class BaseEventHandler;
        }

        namespace connection {
            class Connection;
        }
        namespace spi {

            class ClientContext;

            class HAZELCAST_API InvocationService {
            public:
                InvocationService(spi::ClientContext &clientContext);

                void start();

                boost::shared_future<serialization::Data> invokeOnRandomTarget(const impl::PortableRequest *request);

                boost::shared_future<serialization::Data> invokeOnKeyOwner(const impl::PortableRequest *request, serialization::Data &key);

                boost::shared_future<serialization::Data> invokeOnTarget(const impl::PortableRequest *request, const Address &target);

                boost::shared_future<serialization::Data> invokeOnRandomTarget(const impl::PortableRequest *request, impl::BaseEventHandler *handler);

                boost::shared_future<serialization::Data> invokeOnTarget(const impl::PortableRequest *request, impl::BaseEventHandler *handler, const Address &target);

                boost::shared_future<serialization::Data> invokeOnKeyOwner(const impl::PortableRequest *request, impl::BaseEventHandler *handler, const serialization::Data &key);

                bool resend(boost::shared_ptr<util::CallPromise> promise);

                bool isRedoOperation() const;

                static const int RETRY_COUNT = 20;
                static const int RETRY_WAIT_TIME = 500;
            private :
                bool redoOperation;
                spi::ClientContext &clientContext;

                boost::shared_future<serialization::Data> doSend(const impl::PortableRequest *request, impl::BaseEventHandler *eventHandler, connection::Connection &connection);

                boost::shared_ptr<connection::Connection> getOrConnect(const Address *target);

            };
        }
    }
}


#endif //__InvocationService_H_
