//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#ifndef HAZELCAST_CLUSTER_LISTENER_THREAD
#define HAZELCAST_CLUSTER_LISTENER_THREAD

#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/Thread.h"
#include <boost/shared_ptr.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

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
            class Connection;

            class ConnectionManager;

            class HAZELCAST_API ClusterListenerThread {
            public:
                ClusterListenerThread(spi::ClientContext &clientContext);

                void setThread(hazelcast::util::Thread *);

                static void staticRun(hazelcast::util::ThreadArgs &args);

                void run(hazelcast::util::Thread *currentThread);

                void stop();

                std::vector<Address> getSocketAddresses();

                hazelcast::util::CountDownLatch startLatch;
                bool isStartedSuccessfully;
            private:
                spi::ClientContext &clientContext;
                boost::shared_ptr<Connection> conn;
                hazelcast::util::AtomicBoolean deletingConnection;
                std::vector<Member> members;

                std::auto_ptr<hazelcast::util::Thread> clusterListenerThread;

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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLUSTER_LISTENER_THREAD

