//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/impl/ClientRequest.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/CallPromise.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/impl/BaseEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            InvocationService::InvocationService(spi::ClientContext& clientContext)
            : clientContext(clientContext) {

            }

            void InvocationService::start() {
                redoOperation = clientContext.getClientConfig().isRedoOperation();
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::invokeOnRandomTarget(const impl::ClientRequest *request) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getRandomConnection(RETRY_COUNT);
                return doSend(managedRequest, std::auto_ptr<impl::BaseEventHandler>(NULL), connection);
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::invokeOnKeyOwner(const impl::ClientRequest *request, int partitionId) {
                boost::shared_ptr<Address> owner = clientContext.getPartitionService().getPartitionOwner(partitionId);
                if (owner.get() != NULL) {
                    return invokeOnTarget(request, *owner);
                }
                return invokeOnRandomTarget(request);
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::invokeOnTarget(const impl::ClientRequest *request, const Address& address) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(address, RETRY_COUNT);
                return doSend(managedRequest, std::auto_ptr<impl::BaseEventHandler>(NULL), connection);
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::invokeOnRandomTarget(const impl::ClientRequest *request, impl::BaseEventHandler *eventHandler) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                std::auto_ptr<impl::BaseEventHandler> managedEventHandler(eventHandler);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getRandomConnection(RETRY_COUNT);
                return doSend(managedRequest, managedEventHandler, connection);
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::invokeOnTarget(const impl::ClientRequest *request, impl::BaseEventHandler *eventHandler, const Address& address) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                std::auto_ptr<impl::BaseEventHandler> managedEventHandler(eventHandler);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(address, RETRY_COUNT);
                return doSend(managedRequest, managedEventHandler, connection);
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::invokeOnKeyOwner(const impl::ClientRequest *request, impl::BaseEventHandler *handler, int partitionId) {
                boost::shared_ptr<Address> owner = clientContext.getPartitionService().getPartitionOwner(partitionId);
                if (owner.get() != NULL) {
                    return invokeOnTarget(request, handler, *owner);
                }
                return invokeOnRandomTarget(request, handler);
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::invokeOnConnection(const impl::ClientRequest *request, boost::shared_ptr<connection::Connection> connection) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                return doSend(managedRequest, std::auto_ptr<impl::BaseEventHandler>(NULL), connection);
            }

            bool InvocationService::isRedoOperation() const {
                return redoOperation;
            }

            boost::shared_ptr<util::Future<serialization::pimpl::Data> >  InvocationService::doSend(std::auto_ptr<const impl::ClientRequest> request, std::auto_ptr<impl::BaseEventHandler> eventHandler, boost::shared_ptr<connection::Connection> connection) {
                boost::shared_ptr<connection::CallPromise> promise(new connection::CallPromise());
                promise->setRequest(request);
                promise->setEventHandler(eventHandler);
                connection->registerAndEnqueue(promise);
                return promise->getFuture();
            }

        }
    }
}
