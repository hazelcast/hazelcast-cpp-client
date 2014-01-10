//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/impl/PortableRequest.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/util/CallPromise.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            InvocationService::InvocationService(spi::ClientContext &clientContext)
            : clientContext(clientContext)
            , redoOperation(clientContext.getClientConfig().isRedoOperation()) {

            }

            boost::shared_future<serialization::Data> InvocationService::invokeOnRandomTarget(const impl::PortableRequest &request) {
                connection::Connection *connection = getOrConnect(NULL);
                return doSend(request, NULL, *connection);
            };

            boost::shared_future<serialization::Data> InvocationService::invokeOnKeyOwner(const impl::PortableRequest &request, serialization::Data &key) {
                Address *owner = clientContext.getPartitionService().getPartitionOwner(key);
                if (owner != NULL) {
                    return invokeOnTarget(request, *owner);
                }
                return invokeOnRandomTarget(request);
            };

            boost::shared_future<serialization::Data> InvocationService::invokeOnTarget(const impl::PortableRequest &request, const Address &address) {
                connection::Connection *connection = getOrConnect(&address);
                return doSend(request, NULL, *connection);
            };

            boost::shared_future<serialization::Data> InvocationService::invokeOnRandomTarget(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler) {
                connection::Connection *connection = getOrConnect(NULL);
                return doSend(request, eventHandler, *connection);
            }

            boost::shared_future<serialization::Data> InvocationService::invokeOnTarget(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, const Address &address) {
                connection::Connection *connection = getOrConnect(&address);
                return doSend(request, eventHandler, *connection);
            }

            boost::shared_future<serialization::Data> InvocationService::invokeOnKeyOwner(const impl::PortableRequest &request, impl::EventHandlerWrapper *handler, const serialization::Data &key) {
                Address *owner = clientContext.getPartitionService().getPartitionOwner(key);
                if (owner != NULL) {
                    return invokeOnTarget(request, handler, *owner);
                }
                return invokeOnRandomTarget(request, handler);
            }

            void InvocationService::resend(util::CallPromise *promise) {
                connection::Connection *connection = getOrConnect(NULL);
                connection->resend(promise);
            }


            bool InvocationService::isRedoOperation() const {
                return redoOperation;
            }

            boost::shared_future<serialization::Data> InvocationService::doSend(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, connection::Connection &connection) {
                util::CallPromise *promise = new util::CallPromise(*this);
                promise->setRequest(&request);
                promise->setEventHandler(eventHandler);
                connection.send(promise);
                return promise->getFuture();
            }

            connection::Connection *InvocationService::getOrConnect(const Address *target) {
                int count = 0;
                exception::IOException lastError("", "");
                connection::ConnectionManager &connectionManager = clientContext.getConnectionManager();
                while (count < RETRY_COUNT) {
                    try {
                        if (target == NULL || !clientContext.getClusterService().isMemberExists(*target)) {
                            return connectionManager.getRandomConnection();
                        } else {
                            return connectionManager.getOrConnect(*target);
                        }
                    } catch (exception::IOException &e) {
                        lastError = e;
                    }
                    target = NULL;
                    count++;
                }
                throw lastError;
            }
        }
    }
}