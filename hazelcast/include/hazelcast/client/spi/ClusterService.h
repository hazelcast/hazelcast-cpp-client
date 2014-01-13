//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "hazelcast/client/connection/ClusterListenerThread.h"
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
            class ClientContext;

            class HAZELCAST_API ClusterService {
            public:
                friend class connection::ClusterListenerThread;

                ClusterService(ClientContext &clientContext);

                void start();

                void stop();

                std::auto_ptr<Address> getMasterAddress();

                void addMembershipListener(MembershipListener *listener);

                bool removeMembershipListener(MembershipListener *listener);

                bool isMemberExists(const Address &address);

                connection::Member getMember(const std::string &uuid);

                connection::Member getMember(Address &address);

                std::vector<connection::Member> getMemberList();

            private:
                ClientContext &clientContext;

                connection::ClusterListenerThread clusterThread;

                std::map<Address, connection::Member, addressComparator > members;
                std::set< MembershipListener *> listeners;
                boost::mutex listenerLock;
                boost::mutex membersLock;

                boost::atomic<bool> active;

                //--------- Used by CLUSTER LISTENER THREAD ------------
                void fireMembershipEvent(connection::MembershipEvent &membershipEvent);

                void setMembers(const std::map<Address, connection::Member, addressComparator > &map);

                connection::Connection* connectToOne(const std::vector<Address> &socketAddresses);
                // ------------------------------------------------------

            };

        }
    }
}

#endif //HAZELCAST_CLUSTER_SERVICE
