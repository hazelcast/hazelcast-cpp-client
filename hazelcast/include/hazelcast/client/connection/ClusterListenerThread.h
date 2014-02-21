//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_CLUSTER_LISTENER_THREAD
#define HAZELCAST_CLUSTER_LISTENER_THREAD

#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/util/CountDownLatch.h"
#include <boost/atomic.hpp>
#include <boost/thread.hpp>

namespace hazelcast {
    namespace client {

        class Member;

        namespace spi {
            class ClientContext;
        }

        namespace impl {
            class ClientMembershipEvent;

            class MemberAttributeChange;
        }

        namespace connection {

            class ConnectionManager;

            class HAZELCAST_API ClusterListenerThread {
            public:
                ClusterListenerThread(spi::ClientContext &clientContext);

                void setThread(boost::thread *);

                void run();

                void stop();

                util::CountDownLatch startLatch;
            private:
                spi::ClientContext &clientContext;

                std::auto_ptr<boost::thread> clusterListenerThread;

                std::auto_ptr<Connection> conn;
                boost::atomic<bool> deletingConnection;
                std::vector<Member> members;

                connection::Connection *pickConnection();

                void loadInitialMemberList();

                void listenMembershipEvents();

                void updateMembersRef();

                void fireMemberAttributeEvent(impl::MemberAttributeChange const &, Member &member);

                std::vector<Address> getClusterAddresses() const;

                std::vector<Address> getConfigAddresses() const;
            };
        }
    }
}

#endif //HAZELCAST_CLUSTER_LISTENER_THREAD
