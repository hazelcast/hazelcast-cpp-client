/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 5/23/13.


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

                void setThread(util::Thread *);

                static void staticRun(util::ThreadArgs &args);

                void run(util::Thread *currentThread);

                void stop();

                std::vector<Address> getSocketAddresses();

                util::CountDownLatch startLatch;
                bool isStartedSuccessfully;
            private:
                spi::ClientContext &clientContext;
                boost::shared_ptr<Connection> conn;
                util::AtomicBoolean deletingConnection;
                std::vector<Member> members;

                std::auto_ptr<util::Thread> clusterListenerThread;

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

