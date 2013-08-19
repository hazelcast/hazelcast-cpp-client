//
// Created by sancar koyunlu on 8/19/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SmartConnectionManager
#define HAZELCAST_SmartConnectionManager


namespace hazelcast {
    namespace client {
        namespace connection {
            class SmartConnectionManager : public ConnectionManager {
            public:
                SmartConnectionManager(spi::ClusterService& clusterService, serialization::SerializationService&, ClientConfig&);

                ~SmartConnectionManager();

                Connection *newConnection(const Address& address) = 0;

                Connection *getRandomConnection() = 0;

                Connection *getConnection(const Address& address) = 0;

                void releaseConnection(Connection *connection) = 0;

                util::AtomicPointer <ConnectionPool> getConnectionPool(const Address& address) = 0;

                void removeConnectionPool(const Address& address) = 0;

                void authenticate(Connection& connection, bool reAuth, bool firstConnection) = 0;

            private:
                util::ConcurrentSmartMap<Address, ConnectionPool, addressComparator> poolMap;
                spi::ClusterService& clusterService;
                serialization::SerializationService& serializationService;
                ClientConfig& clientConfig;
                protocol::Principal *principal;
                HeartBeatChecker heartBeatChecker;
                std::auto_ptr<connection::SocketInterceptor> socketInterceptor;

            };
        }
    }
}

#endif //HAZELCAST_SmartConnectionManager
