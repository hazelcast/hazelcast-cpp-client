/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <unordered_set>
#include <memory>

#include <boost/thread/latch.hpp>
#include <boost/smart_ptr/atomic_shared_ptr.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "ClientClusterServiceImpl.h"
#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
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
                        : public protocol::codec::ClientAddMembershipListenerCodec::AbstractEventHandler,
                          public std::enable_shared_from_this<ClientMembershipListener> {
                public:
                    ClientMembershipListener(ClientContext &client);

                    void handleMemberEventV10(const Member &member, const int32_t &eventType) override;

                    void handleMemberListEventV10(const std::vector<Member> &members) override;

                    void handleMemberAttributeChangeEventV10(const std::string &uuid, const std::string &key,
                                                                     const int32_t &operationType,
                                                                     std::unique_ptr<std::string> &value) override;

                    void listenMembershipEvents(const std::shared_ptr<connection::Connection> &ownerConnection);

                private:
                    static int INITIAL_MEMBERS_TIMEOUT_SECONDS;

                    ClientContext &client;
                    std::unordered_set<Member> members;
                    ClientClusterServiceImpl &clusterService;
                    ClientPartitionServiceImpl &partitionService;
                    connection::ClientConnectionManagerImpl &connectionManager;
                    boost::atomic_shared_ptr<boost::latch> initialListFetchedLatch;

                    void memberAdded(const Member &member);

                    std::string membersString() const;

                    void memberRemoved(const Member &member);

                    std::exception_ptr newTargetDisconnectedExceptionCausedByMemberLeftEvent(
                            const std::shared_ptr<connection::Connection> &connection);

                    std::vector<MembershipEvent> detectMembershipEvents(std::unordered_map<std::string, Member> &prevMembers);

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
