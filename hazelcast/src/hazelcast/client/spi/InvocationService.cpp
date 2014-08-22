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
#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/ClientProperties.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            InvocationService::InvocationService(spi::ClientContext& clientContext)
            : clientContext(clientContext) {
                redoOperation = clientContext.getClientConfig().isRedoOperation();
                ClientProperties& properties = clientContext.getClientProperties();
                retryWaitTime = properties.getRetryWaitTime().getInteger();
                retryCount = properties.getRetryCount().getInteger();
                heartbeatTimeout = properties.getHeartbeatTimeout().getInteger();
                maxFailedHeartbeatCount = properties.getMaxFailedHeartbeatCount().getInteger();
                if (retryWaitTime <= 0) {
                    retryWaitTime = util::IOUtil::to_value<int>(PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT);
                }
                if (retryCount <= 0) {
                    retryCount = util::IOUtil::to_value<int>(PROP_REQUEST_RETRY_COUNT_DEFAULT);
                }
                if (heartbeatTimeout <= 0) {
                    heartbeatTimeout = util::IOUtil::to_value<int>(PROP_REQUEST_RETRY_WAIT_TIME_DEFAULT);
                }
                if (maxFailedHeartbeatCount <= 0) {
                    maxFailedHeartbeatCount = util::IOUtil::to_value<int>(PROP_MAX_FAILED_HEARTBEAT_COUNT_DEFAULT);
                }
            }

            void InvocationService::start() {

            }

            connection::CallFuture  InvocationService::invokeOnRandomTarget(const impl::ClientRequest *request) {
                return invokeOnRandomTarget(request, NULL);
            }

            connection::CallFuture  InvocationService::invokeOnKeyOwner(const impl::ClientRequest *request, int partitionId) {
                return invokeOnKeyOwner(request, NULL, partitionId);
            }

            connection::CallFuture  InvocationService::invokeOnTarget(const impl::ClientRequest *request, const Address& address) {
                return invokeOnTarget(request, NULL, address);
            }

            connection::CallFuture  InvocationService::invokeOnRandomTarget(const impl::ClientRequest *request, impl::BaseEventHandler *eventHandler) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                std::auto_ptr<impl::BaseEventHandler> managedEventHandler(eventHandler);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getRandomConnection(retryCount);
                return doSend(managedRequest, managedEventHandler, connection, -1);
            }

            connection::CallFuture  InvocationService::invokeOnTarget(const impl::ClientRequest *request, impl::BaseEventHandler *eventHandler, const Address& address) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                std::auto_ptr<impl::BaseEventHandler> managedEventHandler(eventHandler);
                boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(address, retryCount);
                return doSend(managedRequest, managedEventHandler, connection, -1);
            }

            connection::CallFuture  InvocationService::invokeOnKeyOwner(const impl::ClientRequest *request, impl::BaseEventHandler *handler, int partitionId) {
                boost::shared_ptr<Address> owner = clientContext.getPartitionService().getPartitionOwner(partitionId);
                if (owner.get() != NULL) {
                    std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                    std::auto_ptr<impl::BaseEventHandler> managedEventHandler(handler);
                    boost::shared_ptr<connection::Connection> connection = clientContext.getConnectionManager().getOrConnect(*owner, retryCount);
                    return doSend(managedRequest, managedEventHandler, connection, partitionId);
                }
                return invokeOnRandomTarget(request, handler);
            }

            connection::CallFuture  InvocationService::invokeOnConnection(const impl::ClientRequest *request, boost::shared_ptr<connection::Connection> connection) {
                std::auto_ptr<const impl::ClientRequest> managedRequest(request);
                return doSend(managedRequest, std::auto_ptr<impl::BaseEventHandler>(NULL), connection, -1);
            }

            bool InvocationService::isRedoOperation() const {
                return redoOperation;
            }

            connection::CallFuture  InvocationService::doSend(std::auto_ptr<const impl::ClientRequest> request, std::auto_ptr<impl::BaseEventHandler> eventHandler, boost::shared_ptr<connection::Connection> connection, int partitionId) {
                boost::shared_ptr<connection::CallPromise> promise(new connection::CallPromise());
                promise->setRequest(request);
                promise->setEventHandler(eventHandler);
                connection->registerAndEnqueue(promise, partitionId);
                return connection::CallFuture(promise, connection, heartbeatTimeout);
            }

            int InvocationService::getRetryWaitTime() const {
                return retryWaitTime;
            }

            int InvocationService::getRetryCount() const {
                return retryCount;
            }

            int InvocationService::getMaxFailedHeartbeatCount() const {
                return maxFailedHeartbeatCount;
            }
        }
    }
}
