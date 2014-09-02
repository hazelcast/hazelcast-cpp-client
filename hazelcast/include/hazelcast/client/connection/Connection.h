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
#include "hazelcast/util/Closeable.h"

namespace hazelcast {
    namespace client {

        namespace impl {
            class ClientRequest;
        }
        namespace spi {
            class ClientContext;

            class InvocationService;
        }
        namespace serialization {
            namespace pimpl {
                class SerializationService;

                class Data;

                class Packet;

                class PortableContext;
            }

        }

        class Address;

        namespace connection {
            class CallPromise;

            class ClientResponse;

            class OutSelector;

            class InSelector;

            class HAZELCAST_API Connection : public util::Closeable {
            public:
                Connection(const Address& address, spi::ClientContext& clientContext, InSelector& iListener, OutSelector& listener);

                ~Connection();

                void init(const std::vector<byte>& PROTOCOL);

                void connect(int timeoutInMillis);

                void close();

                void write(serialization::pimpl::Packet *packet);

                const Address& getRemoteEndpoint() const;

                void setRemoteEndpoint(Address& remoteEndpoint);

                Socket& getSocket();

                boost::shared_ptr<connection::ClientResponse> sendAndReceive(const impl::ClientRequest& clientRequest);

                ReadHandler& getReadHandler();

                WriteHandler& getWriteHandler();

                void setAsOwnerConnection(bool isOwnerConnection);

                void writeBlocking(serialization::pimpl::Packet const& packet);

                serialization::pimpl::Packet readBlocking();

                bool isHeartBeating();

                void heartBeatingFailed();

                void heartBeatingSucceed();

                util::AtomicInt lastRead;
                util::AtomicInt lastWrite;
                util::AtomicBoolean live;
            private:
                spi::ClientContext& clientContext;
                spi::InvocationService& invocationService;
                Socket socket;
                ReadHandler readHandler;
                WriteHandler writeHandler;
                bool _isOwnerConnection;

                util::AtomicBoolean heartBeating;


            };

        }
    }
}


#endif //HAZELCAST_CONNECTION

