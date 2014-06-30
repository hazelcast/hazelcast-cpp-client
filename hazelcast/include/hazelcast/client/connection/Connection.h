//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/AtomicInt.h"

namespace hazelcast {
    namespace client {

        namespace spi {
            class ClientContext;
        }
        namespace serialization {
            namespace pimpl {
                class SerializationService;

                class Data;
            }

        }

        class Address;

        namespace connection {
            class CallPromise;

            class ClientResponse;

            class OutSelector;

            class InSelector;

            class HAZELCAST_API Connection {
            public:
                Connection(const Address &address, spi::ClientContext &clientContext, InSelector &iListener, OutSelector &listener);

                ~Connection();

                void init(const std::vector<byte> &PROTOCOL);

                void connect(int timeoutInMillis);

                void close();

                void registerAndEnqueue(boost::shared_ptr<CallPromise> promise);

                void handlePacket(const serialization::pimpl::Data &data);

                const Address &getRemoteEndpoint() const;

                void setRemoteEndpoint(Address &remoteEndpoint);

                const Socket &getSocket() const;

                void writeBlocking(serialization::pimpl::Data const &data);

                serialization::pimpl::Data readBlocking();

                ReadHandler &getReadHandler();

                WriteHandler &getWriteHandler();

                boost::shared_ptr<CallPromise> deRegisterEventHandler(int callId);

                void setAsOwnerConnection(bool isOwnerConnection);

                util::AtomicInt lastRead;
                util::AtomicInt lastWrite;
                util::AtomicBoolean live;
            private:
                spi::ClientContext &clientContext;
                Socket socket;
                util::SynchronizedMap<int, CallPromise > callPromises;
                util::SynchronizedMap<int, CallPromise > eventHandlerPromises;
                ReadHandler readHandler;
                WriteHandler writeHandler;
                bool _isOwnerConnection;

                void removeConnectionCalls();

                boost::shared_ptr<CallPromise> deRegisterCall(int callId);

                void registerEventHandler(boost::shared_ptr<CallPromise> promise);

                boost::shared_ptr<CallPromise> getEventHandlerPromise(int callId);

                void resend(boost::shared_ptr<CallPromise> promise);

                void registerCall(boost::shared_ptr<CallPromise> promise);

                void targetNotActive(boost::shared_ptr<CallPromise> promise);

                bool handleEventUuid(boost::shared_ptr<ClientResponse> response, boost::shared_ptr<CallPromise> promise);

                bool handleException(boost::shared_ptr<ClientResponse> response, boost::shared_ptr<CallPromise> promise);

            };

        }
    }
}


#endif //HAZELCAST_CONNECTION

