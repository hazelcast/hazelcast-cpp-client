//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/ReadHandler.h"
#include "hazelcast/client/connection/WriteHandler.h"
#include "hazelcast/util/SynchronizedMap.h"

namespace hazelcast {
    namespace util {
        class CallPromise;
    }
    namespace client {
        namespace connection {
            class ClientResponse;
        }

        namespace spi {
            class ClientContext;
        }
        namespace serialization {
            class SerializationService;

            class Data;
        }

        class Address;

        namespace connection {
            class OListener;

            class IListener;

            class HAZELCAST_API Connection {
            public:
                Connection(const Address &address, spi::ClientContext &clientContext, IListener &iListener, OListener &listener);

                void connect();

                void init();

                void close();

                void send(boost::shared_ptr<util::CallPromise> promise);

                void resend(boost::shared_ptr<util::CallPromise> promise);

                void handlePacket(const serialization::Data &data);

                const Address &getRemoteEndpoint() const;

                void setRemoteEndpoint(Address &remoteEndpoint);

                const Socket &getSocket() const;

                void writeBlocking(serialization::Data const &data);

                serialization::Data readBlocking();

                ReadHandler &getReadHandler();

                // USED BY CLUSTER SERVICE

                boost::shared_ptr<util::CallPromise> deRegisterCall(int callId);

                void registerEventHandler(boost::shared_ptr<util::CallPromise> promise);

                boost::shared_ptr<util::CallPromise> getEventHandlerPromise(int callId);

                boost::shared_ptr<util::CallPromise> deRegisterEventHandler(int callId);

                void removeConnectionCalls();

                boost::atomic<clock_t> lastRead;
                boost::atomic<clock_t> lastWrite;
                boost::atomic<bool> live;
            private:
                spi::ClientContext &clientContext;
                Socket socket;
                Address remoteEndpoint;
                util::SynchronizedMap<int, util::CallPromise > callPromises;
                util::SynchronizedMap<int, util::CallPromise > eventHandlerPromises;
                ReadHandler readHandler;
                WriteHandler writeHandler;

                void write(boost::shared_ptr<util::CallPromise> promise);

                boost::shared_ptr<util::CallPromise> registerCall(boost::shared_ptr<util::CallPromise> promise);

                void reRegisterCall(boost::shared_ptr<util::CallPromise> promise);

                void targetNotActive(boost::shared_ptr<util::CallPromise> promise);

                bool handleEventUuid(boost::shared_ptr<connection::ClientResponse> response, boost::shared_ptr<util::CallPromise> promise);

            };

        }
    }
}


#endif //HAZELCAST_CONNECTION
