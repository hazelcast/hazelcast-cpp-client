//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/util/CallPromise.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            InvocationService::InvocationService(spi::ClientContext &clientContext)
            : clientContext(clientContext) {

            }

            void InvocationService::start() {
                redoOperation = clientContext.getClientConfig().isRedoOperation();
            }

            boost::shared_future<serialization::Data> InvocationService::invokeOnRandomTarget(const impl::PortableRequest *request) {
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getRandomConnection(RETRY_COUNT);
                return doSend(request, NULL, connection);
            };

            boost::shared_future<serialization::Data> InvocationService::invokeOnKeyOwner(const impl::PortableRequest *request, serialization::Data &key) {
                boost::shared_ptr<Address> owner = clientContext.getPartitionService().getPartitionOwner(key);
                if (owner.get() != NULL) {
                    return invokeOnTarget(request, *owner);
                }
                return invokeOnRandomTarget(request);
            };

            boost::shared_future<serialization::Data> InvocationService::invokeOnTarget(const impl::PortableRequest *request, const Address &address) {
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(address, RETRY_COUNT);
                return doSend(request, NULL, connection);
            };

            boost::shared_future<serialization::Data> InvocationService::invokeOnRandomTarget(const impl::PortableRequest *request, impl::BaseEventHandler *eventHandler) {
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getRandomConnection(RETRY_COUNT);
                return doSend(request, eventHandler, connection);
            }

            boost::shared_future<serialization::Data> InvocationService::invokeOnTarget(const impl::PortableRequest *request, impl::BaseEventHandler *eventHandler, const Address &address) {
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(address, RETRY_COUNT);
                return doSend(request, eventHandler, connection);
            }

            boost::shared_future<serialization::Data> InvocationService::invokeOnKeyOwner(const impl::PortableRequest *request, impl::BaseEventHandler *handler, const serialization::Data &key) {
                boost::shared_ptr<Address> owner = clientContext.getPartitionService().getPartitionOwner(key);
                if (owner.get() != NULL) {
                    return invokeOnTarget(request, handler, *owner);
                }
                return invokeOnRandomTarget(request, handler);
            }

            boost::shared_future<serialization::Data> InvocationService::invokeOnConnection(const impl::PortableRequest *request, boost::shared_ptr<connection::Connection> connection) {
                return doSend(request, NULL, connection);
            }

            bool InvocationService::isRedoOperation() const {
                return redoOperation;
            }

            boost::shared_future<serialization::Data> InvocationService::doSend(const impl::PortableRequest *request, impl::BaseEventHandler *eventHandler, boost::shared_ptr<connection::Connection> connection) {
                boost::shared_ptr<util::CallPromise> promise(new util::CallPromise(*this));
                promise->setRequest(request);
                promise->setEventHandler(eventHandler);
                connection->send(promise);
                return promise->getFuture();
            }

        }
    }
}