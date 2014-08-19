//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/Future.h"
#include <boost/shared_ptr.hpp>

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

            class CallPromise;
        }
        namespace spi {

            class ClientContext;

            class HAZELCAST_API InvocationService {
            public:
                InvocationService(spi::ClientContext &clientContext);

                void start();

                boost::shared_ptr< util::Future<serialization::pimpl::Data> > invokeOnRandomTarget(const impl::ClientRequest *request);

                boost::shared_ptr< util::Future<serialization::pimpl::Data> > invokeOnKeyOwner(const impl::ClientRequest *request, int partitionId);

                boost::shared_ptr< util::Future<serialization::pimpl::Data> > invokeOnTarget(const impl::ClientRequest *request, const Address &target);

                boost::shared_ptr< util::Future<serialization::pimpl::Data> > invokeOnRandomTarget(const impl::ClientRequest *request, impl::BaseEventHandler *handler);

                boost::shared_ptr< util::Future<serialization::pimpl::Data> > invokeOnTarget(const impl::ClientRequest *request, impl::BaseEventHandler *handler, const Address &target);

                boost::shared_ptr< util::Future<serialization::pimpl::Data> > invokeOnKeyOwner(const impl::ClientRequest *request, impl::BaseEventHandler *handler, int partitionId);

                boost::shared_ptr< util::Future<serialization::pimpl::Data> > invokeOnConnection(const impl::ClientRequest *request, boost::shared_ptr<connection::Connection> connection);

                bool isRedoOperation() const;

                static const int RETRY_COUNT = 20;
            private :
                bool redoOperation;
                spi::ClientContext &clientContext;

                boost::shared_ptr< util::Future<serialization::pimpl::Data> >  doSend(std::auto_ptr<const impl::ClientRequest>, std::auto_ptr<impl::BaseEventHandler>, boost::shared_ptr<connection::Connection>, int);


            };
        }
    }
}


#endif //__InvocationService_H_

