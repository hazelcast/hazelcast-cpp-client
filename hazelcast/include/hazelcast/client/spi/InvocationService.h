/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/23/13.


#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/SynchronizedMap.h"
#include <boost/shared_ptr.hpp>
#include "hazelcast/client/protocol/IMessageHandler.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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

            class CallPromise;
        }

        namespace protocol {
            class ClientMessage;
        }

        namespace spi {

            class ClientContext;

            class HAZELCAST_API InvocationService : public protocol::IMessageHandler {
            public:
                InvocationService(spi::ClientContext& clientContext);

                virtual ~InvocationService();

                void start();

                connection::CallFuture invokeOnRandomTarget(std::auto_ptr<protocol::ClientMessage> request);

                connection::CallFuture invokeOnPartitionOwner(std::auto_ptr<protocol::ClientMessage> request, int partitionId);

                connection::CallFuture invokeOnTarget(std::auto_ptr<protocol::ClientMessage> request, const Address& target);

                connection::CallFuture invokeOnRandomTarget(std::auto_ptr<protocol::ClientMessage> request, hazelcast::client::impl::BaseEventHandler *handler);

                connection::CallFuture invokeOnTarget(std::auto_ptr<protocol::ClientMessage> request, hazelcast::client::impl::BaseEventHandler *handler, const Address& target);

                connection::CallFuture invokeOnPartitionOwner(std::auto_ptr<protocol::ClientMessage> request, hazelcast::client::impl::BaseEventHandler *handler, int partitionId);

                connection::CallFuture invokeOnConnection(std::auto_ptr<protocol::ClientMessage> request, boost::shared_ptr<connection::Connection> connection);

                bool isRedoOperation() const;

                int getRetryWaitTime() const;

                int getRetryCount() const;

                void handleMessage(connection::Connection &connection, std::auto_ptr<protocol::ClientMessage> message);

                /**
                * Removes event handler corresponding to callId from responsible ClientConnection
                *
                * @param callId of event handler registration request
                * @return true if found and removed, false otherwise
                */
                void removeEventHandler(int callId);

                /**
                * Clean all promises (both request and event handlers). Retries requests on available connections if applicable.
                */
                void cleanResources(connection::Connection& connection);

                /**
                * Cleans just event handler promises, Retries requests on available connections if applicable.
                */
                void cleanEventHandlers(connection::Connection& connection);

                /**
                *  Retries the given promise on an available connection if request is retryable.
                */
                void tryResend(boost::shared_ptr<connection::CallPromise> promise, const std::string& lastTriedAddress);

                /**
                *  Retries the given promise on an available connection.
                */
                boost::shared_ptr<connection::Connection> resend(boost::shared_ptr<connection::CallPromise> promise, const std::string& lastAddress);
            private:
                bool redoOperation;
                int heartbeatTimeout;
                int retryWaitTime;
                int retryCount;
                spi::ClientContext& clientContext;
                util::SynchronizedMap<connection::Connection* , util::SynchronizedMap<int, connection::CallPromise > > callPromises;
                util::SynchronizedMap<connection::Connection* , util::SynchronizedMap<int, connection::CallPromise > > eventHandlerPromises;

                bool isAllowedToSentRequest(connection::Connection& connection, protocol::ClientMessage const&);

                connection::CallFuture doSend(std::auto_ptr<protocol::ClientMessage> request, std::auto_ptr<hazelcast::client::impl::BaseEventHandler> eventHandler, boost::shared_ptr<connection::Connection>, int);

                /**
                * Returns the actual connection that request is send over,
                * Returns null shared_ptr if request is not send.
                */
                boost::shared_ptr<connection::Connection> registerAndEnqueue(boost::shared_ptr<connection::Connection>,boost::shared_ptr<connection::CallPromise>, int partitionId);

                /** CallId Related **/

                void registerCall(connection::Connection& connection, boost::shared_ptr<connection::CallPromise> promise);

                boost::shared_ptr<connection::CallPromise> deRegisterCall(connection::Connection& connection, int callId);

                /** **/
                void registerEventHandler(int correlationId,
                                          connection::Connection& connection, boost::shared_ptr<connection::CallPromise> promise);

                boost::shared_ptr<connection::CallPromise> deRegisterEventHandler(connection::Connection& connection, int callId);

                /***** HANDLE PACKET PART ****/

                /* returns shouldSetResponse */
                bool handleException(protocol::ClientMessage *response, boost::shared_ptr<connection::CallPromise> promise, const  std::string& address);

                /* returns shouldSetResponse */
                bool handleEventUuid(protocol::ClientMessage *response, boost::shared_ptr<connection::CallPromise> promise);

                /** CallPromise Map **/

                boost::shared_ptr< util::SynchronizedMap<int, connection::CallPromise> > getCallPromiseMap(connection::Connection& connection);

                /** EventHandler Map **/

                // TODO: Put the promise map as a member of the connection object. In this way, we can get the promise map directly from connection object
                // without a need for a map lookup since we already know the connection and the map is specific to a connection
                boost::shared_ptr< util::SynchronizedMap<int, connection::CallPromise> > getEventHandlerPromiseMap(connection::Connection& connection);

                boost::shared_ptr<connection::CallPromise> getEventHandlerPromise(connection::Connection& , int callId);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__InvocationService_H_

