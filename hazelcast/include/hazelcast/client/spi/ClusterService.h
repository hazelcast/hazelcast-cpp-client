//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ResponseStream.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/connection/ClusterListenerThread.h"
#include "hazelcast/client/connection/ConnectionManager.h"
#include "hazelcast/client/exception/ServerException.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/util/ConcurrentSmartMap.h"
#include "hazelcast/util/AtomicPointer.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/serialization/DataAdapter.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/client/connection/MemberShipEvent.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <set>

namespace hazelcast {

    namespace util {
        class CallPromise;
    }

    namespace client {

        namespace impl {
            class PortableRequest;

            class EventHandlerWrapper;
        }
        namespace spi {


            class HAZELCAST_API ClusterService {
            public:
                friend class connection::ClusterListenerThread;

                ClusterService(spi::PartitionService &, spi::LifecycleService &, connection::ConnectionManager &, serialization::SerializationService &, ClientConfig &);

                void start();

                void stop();

                boost::shared_future<serialization::Data> send(const impl::PortableRequest &request);

                boost::shared_future<serialization::Data> send(const impl::PortableRequest &request, const Address &address);

                boost::shared_future<serialization::Data> send(const impl::PortableRequest &request, connection::Connection &connection);

                boost::shared_future<serialization::Data> send(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler);

                boost::shared_future<serialization::Data> send(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, const Address &address);

                boost::shared_future<serialization::Data> send(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, connection::Connection &connection);

                void registerListener(const std::string &uuid, int callId);

                void reRegisterListener(const std::string &uuid, const std::string &alias, int callId);

                bool deRegisterListener(const std::string &uuid);

                void resend(util::CallPromise *promise);

                std::auto_ptr<Address> getMasterAddress();

                void addMembershipListener(MembershipListener *listener);

                bool removeMembershipListener(MembershipListener *listener);

                bool isMemberExists(const Address &address);

                connection::Member getMember(const std::string &uuid);

                connection::Member getMember(Address &address);

                std::vector<connection::Member> getMemberList();

                void removeConnectionCalls(connection::Connection &connection);

                void handlePacket(const Address &address, serialization::Data &data);

                static const int RETRY_COUNT = 20;
                static const int RETRY_WAIT_TIME = 500;
            private:
                typedef util::SynchronizedMap<int, util::CallPromise > CallMap;
                util::ConcurrentSmartMap<Address, CallMap, addressComparator> addressCallMap;
                util::ConcurrentSmartMap<Address, CallMap, addressComparator> addressEventHandlerMap;
                util::SynchronizedMap<std::string, int > registrationIdMap;
                util::SynchronizedMap<std::string, std::string > registrationAliasMap;
                boost::mutex connectionLock;
                boost::atomic<long> callIdGenerator;
                connection::ConnectionManager &connectionManager;
                serialization::SerializationService &serializationService;
                ClientConfig &clientConfig;
                spi::LifecycleService &lifecycleService;
                spi::PartitionService &partitionService;

                connection::ClusterListenerThread clusterThread;

                protocol::Credentials &credentials;
                std::map<Address, connection::Member, addressComparator > members;
                std::set< MembershipListener *> listeners;
                boost::mutex listenerLock;
                boost::mutex membersLock;
                const bool redoOperation;
                boost::atomic<bool> active;

                util::CallPromise *deRegisterCall(connection::Connection &connection, int callId);

                void registerEventHandler(util::CallPromise *promise, connection::Connection &connection);

                void reRegisterCall(util::CallPromise *promise, connection::Connection &connection);

                void reRegisterEventHandler(util::CallPromise *promise, connection::Connection &connection);

                util::CallPromise *registerCall(const impl::PortableRequest &request, impl::EventHandlerWrapper *handler, connection::Connection &connection);

                connection::Connection *getOrConnect(const Address *target);

                //--------- Used by CLUSTER LISTENER THREAD ------------
                void fireMembershipEvent(connection::MembershipEvent &membershipEvent);

                void setMembers(const std::map<Address, connection::Member, addressComparator > &map);

                connection::Connection *connectToOne(const std::vector<Address> &socketAddresses);
                // ------------------------------------------------------

            };

        }
    }
}

#endif //HAZELCAST_CLUSTER_SERVICE
