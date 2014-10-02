//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_INVOCATION_SERVICE
#define HAZELCAST_INVOCATION_SERVICE

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/SynchronizedMap.h"
#include <boost/shared_ptr.hpp>
#include <hazelcast/client/connection/Connection.h>

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

                class Packet;
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

            class ClientResponse;
        }
        namespace spi {

            class ClientContext;

            class HAZELCAST_API InvocationService {
            public:
                InvocationService(spi::ClientContext& clientContext);

                void start();

                connection::CallFuture invokeOnRandomTarget(const impl::ClientRequest *request);

                connection::CallFuture invokeOnPartitionOwner(const impl::ClientRequest *request, int partitionId);

                connection::CallFuture invokeOnTarget(const impl::ClientRequest *request, const Address& target);

                connection::CallFuture invokeOnRandomTarget(const impl::ClientRequest *request, impl::BaseEventHandler *handler);

                connection::CallFuture invokeOnTarget(const impl::ClientRequest *request, impl::BaseEventHandler *handler, const Address& target);

                connection::CallFuture invokeOnPartitionOwner(const impl::ClientRequest *request, impl::BaseEventHandler *handler, int partitionId);

                connection::CallFuture invokeOnConnection(const impl::ClientRequest *request, boost::shared_ptr<connection::Connection> connection);

                bool isRedoOperation() const;

                int getRetryWaitTime() const;

                int getRetryCount() const;

                void handlePacket(connection::Connection& connection, const serialization::pimpl::Packet& packet);

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
                util::AtomicInt callIdGenerator;
                util::SynchronizedMap<connection::Connection* , util::SynchronizedMap<int, connection::CallPromise > > callPromises;
                util::SynchronizedMap<connection::Connection* , util::SynchronizedMap<int, connection::CallPromise > > eventHandlerPromises;

                int getNextCallId();

                bool isAllowedToSentRequest(connection::Connection& connection, impl::ClientRequest const&);

                connection::CallFuture doSend(std::auto_ptr<const impl::ClientRequest>, std::auto_ptr<impl::BaseEventHandler>, boost::shared_ptr<connection::Connection>, int);

                /**
                * Returns the actual connection that request is send over,
                * Returns null shared_ptr if request is not send.
                */
                boost::shared_ptr<connection::Connection> registerAndEnqueue(boost::shared_ptr<connection::Connection>,boost::shared_ptr<connection::CallPromise>, int partitionId);

                /** CallId Related **/

                void registerCall(connection::Connection& connection, boost::shared_ptr<connection::CallPromise> promise);

                boost::shared_ptr<connection::CallPromise> deRegisterCall(connection::Connection& connection, int callId);

                /** **/
                void registerEventHandler(connection::Connection& connection, boost::shared_ptr<connection::CallPromise> promise);

                boost::shared_ptr<connection::CallPromise> deRegisterEventHandler(connection::Connection& connection, int callId);

                /***** HANDLE PACKET PART ****/

                /* returns shouldSetResponse */
                bool handleException(boost::shared_ptr<connection::ClientResponse> response, boost::shared_ptr<connection::CallPromise> promise, const  std::string& address);

                /* returns shouldSetResponse */
                bool handleEventUuid(boost::shared_ptr<connection::ClientResponse> response, boost::shared_ptr<connection::CallPromise> promise);

                /** CallPromise Map **/

                boost::shared_ptr< util::SynchronizedMap<int, connection::CallPromise> > getCallPromiseMap(connection::Connection& connection);

                /** EventHandler Map **/

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

