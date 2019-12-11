/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTMEMBERSHIPLISTENER_H
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTMEMBERSHIPLISTENER_H

#include <set>
#include <boost/shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "ClientClusterServiceImpl.h"
#include "hazelcast/client/protocol/codec/ClientAddMembershipListenerCodec.h"
#include "hazelcast/util/CountDownLatch.h"
#include "hazelcast/client/MembershipEvent.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        namespace exception {
            class IException;
        }

        namespace connection {
            class ClientConnectionManagerImpl;

            class Connection;
        }

        namespace spi {
            class ClientContext;

            namespace impl {
                class ClientPartitionServiceImpl;
                class ClientClusterServiceImpl;

                class HAZELCAST_API ClientMembershipListener
                        : public protocol::codec::ClientAddMembershipListenerCodec::AbstractEventHandler {
                public:
                    ClientMembershipListener(ClientContext &client);

                    virtual void handleMemberEventV10(const Member &member, const int32_t &eventType);

                    virtual void handleMemberListEventV10(const std::vector<Member> &members);

                    virtual void handleMemberAttributeChangeEventV10(const std::string &uuid, const std::string &key,
                                                             const int32_t &operationType,
                                                             std::auto_ptr<std::string> value);

                    void listenMembershipEvents(const boost::shared_ptr<ClientMembershipListener> &listener,
                                                       const boost::shared_ptr<connection::Connection> &ownerConnection);

                private:
                    static int INITIAL_MEMBERS_TIMEOUT_SECONDS;

                    ClientContext &client;
                    util::ILogger &logger;
                    std::set<Member> members;
                    ClientClusterServiceImpl &clusterService;
                    ClientPartitionServiceImpl &partitionService;
                    connection::ClientConnectionManagerImpl &connectionManager;
                    util::Atomic<boost::shared_ptr<util::CountDownLatch> > initialListFetchedLatch;

                    void memberAdded(const Member &member);

                    std::string membersString() const;

                    void memberRemoved(const Member &member);

                    boost::shared_ptr<exception::IException> newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                            const boost::shared_ptr<connection::Connection> &connection);

                    std::vector<MembershipEvent> detectMembershipEvents(std::map<std::string, Member> &prevMembers);

                    void fireMembershipEvent(std::vector<MembershipEvent> &events);

                    void waitInitialMemberListFetched();
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif 
