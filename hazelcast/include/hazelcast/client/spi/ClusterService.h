//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "hazelcast/client/connection/ClusterListenerThread.h"
#include "hazelcast/util/ConcurrentSmartMap.h"
#include "hazelcast/util/SynchronizedMap.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <set>

namespace hazelcast {

    namespace util {
        class CallPromise;
    }

    namespace client {
        class MembershipListener;

        namespace protocol {
            class Credentials;
        }
        namespace impl {
            class PortableRequest;

            class EventHandlerWrapper;
        }
        namespace spi {
            class PartitionService;

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

                bool isRedoOperation() const;

                void handlePacket(connection::Connection &connection, serialization::Data &data);

                static const int RETRY_COUNT = 20;
                static const int RETRY_WAIT_TIME = 500;
            private:
                util::SynchronizedMap<std::string, int > registrationIdMap;
                util::SynchronizedMap<std::string, const std::string > registrationAliasMap;

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

                connection::Connection *getOrConnect(const Address *target);

                boost::shared_future<serialization::Data> doSend(const impl::PortableRequest &request, impl::EventHandlerWrapper *eventHandler, connection::Connection &connection);

                void _send(util::CallPromise *promise, connection::Connection &connection);

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
