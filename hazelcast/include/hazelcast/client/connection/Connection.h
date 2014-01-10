//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONNECTION
#define HAZELCAST_CONNECTION

#include "hazelcast/client/Socket.h"
#include "hazelcast/client/connection/InputHandler.h"
#include "hazelcast/client/connection/OutputHandler.h"
#include "hazelcast/util/SynchronizedMap.h"

namespace hazelcast {
    namespace util {
        class CallPromise;
    }
    namespace client {
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

            static int CONN_ID = 1;

            class HAZELCAST_API Connection {
            public:
                Connection(const Address &address, spi::ClientContext& clientContext, IListener &iListener, OListener &listener);

                void connect();

                void init();

                void close();

                void send(util::CallPromise *promise);

                void resend(util::CallPromise *promise);

                void handlePacket(const serialization::Data &data);

                const Address &getRemoteEndpoint() const;

                void setRemoteEndpoint(Address &remoteEndpoint);

                int getConnectionId() const;

                const Socket &getSocket() const;

                void writeBlocking(serialization::Data const &data);

                serialization::Data readBlocking();

                // USED BY CLUSTER SERVICE

                util::CallPromise *deRegisterCall(int callId);

                void registerEventHandler(util::CallPromise *promise);

                util::CallPromise *getEventHandler(int callId);

                util::CallPromise *deRegisterEventHandler(int callId);

                void removeConnectionCalls();

                boost::atomic<clock_t> lastRead;
                boost::atomic<clock_t> lastWrite;
                boost::atomic<bool> live;
            private:
                spi::ClientContext& clientContext;
                Socket socket;
                int connectionId;
                Address remoteEndpoint;
                util::SynchronizedMap<int, util::CallPromise > callPromises;
                util::SynchronizedMap<int, util::CallPromise > eventHandlerPromises;
                InputHandler readHandler;
                OutputHandler writeHandler;

                void write(util::CallPromise *promise);

                util::CallPromise *registerCall(util::CallPromise *promise);

                void reRegisterCall(util::CallPromise *promise);
            };

            inline std::ostream &operator <<(std::ostream &strm, const Connection &a) {
                return strm << "Connection [id " << a.getConnectionId() << "][" << a.getRemoteEndpoint()
                        << " -> " << a.getSocket().getAddress() << "]";
            };
        }
    }
}


#endif //HAZELCAST_CONNECTION
