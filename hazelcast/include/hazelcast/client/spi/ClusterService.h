//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#ifndef HAZELCAST_CLUSTER_SERVICE
#define HAZELCAST_CLUSTER_SERVICE

#include "hazelcast/client/connection/ClusterListenerThread.h"
#include "hazelcast/util/Mutex.h"
#include "hazelcast/util/LockGuard.h"
#include <set>

namespace hazelcast {

    namespace client {
        class MembershipListener;

        class MembershipEvent;

        class InitialMembershipListener;

        class MemberAttributeEvent;

        namespace protocol {
            class UsernamePasswordCredentials;
        }
        namespace impl {
            class ClientRequest;

            class BaseEventHandler;
        }
        namespace spi {
            class ClientContext;

            class HAZELCAST_API ClusterService {
            public:
                friend class connection::ClusterListenerThread;

                ClusterService(ClientContext &clientContext);

                bool start();

                void stop();

                std::auto_ptr<Address> getMasterAddress();

                void addMembershipListener(MembershipListener *listener);

                void addMembershipListener(InitialMembershipListener *listener);

                bool removeMembershipListener(MembershipListener *listener);

                bool removeMembershipListener(InitialMembershipListener *listener);

                bool isMemberExists(const Address &address);

                Member getMember(const std::string &uuid);

                Member getMember(Address &address);

                std::vector<Member> getMemberList();

            private:
                ClientContext &clientContext;

                connection::ClusterListenerThread clusterThread;

                std::map<Address, Member, addressComparator > members;
                std::set< MembershipListener *> listeners;
                std::set< InitialMembershipListener *> initialListeners;
                util::Mutex listenerLock;
                util::Mutex membersLock;

                util::AtomicBoolean active;

                void initMembershipListeners();

                //--------- Used by CLUSTER LISTENER THREAD ------------
                void fireMembershipEvent(MembershipEvent &membershipEvent);

                void fireMemberAttributeEvent(MemberAttributeEvent &membershipEvent);

                void setMembers(const std::map<Address, Member, addressComparator > &map);

                connection::Connection *connectToOne(const std::vector<Address> &socketAddresses);
                // ------------------------------------------------------

            };

        }
    }
}

#endif //HAZELCAST_CLUSTER_SERVICE

