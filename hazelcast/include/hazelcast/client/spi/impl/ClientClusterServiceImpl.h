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

#include <unordered_map>
#include <vector>

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/client/Client.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace connection {
            class Connection;
        }
        class InitialMembershipListener;
        class InitialMembershipEvent;

        namespace spi {
            class ClientContext;

            namespace impl {
                class ClientMembershipListener;

                class HAZELCAST_API ClientClusterServiceImpl : public ClientClusterService {
                public:
                    ClientClusterServiceImpl(ClientContext &client);

                    void start();

                    void shutdown();

                    boost::optional<Member> getMember(const Address &address) override;

                    boost::optional<Member> getMember(const std::string &uuid) override;

                    std::vector<Member> getMemberList() override;

                    std::vector<Member> getMembers(
                            const cluster::memberselector::MemberSelector &selector) override;

                    std::string addMembershipListener(const std::shared_ptr<MembershipListener> &listener) override;

                    bool removeMembershipListener(const std::string &registrationId) override;

                    void handleMembershipEvent(const MembershipEvent &event);

                    void handleInitialMembershipEvent(const InitialMembershipEvent &event);

                    void listenMembershipEvents(const std::shared_ptr<connection::Connection> &ownerConnection);

                    void fireMemberAttributeEvent(const MemberAttributeEvent &event);

                    int getSize() override;

                    Client getLocalClient() const;

                private:
                    ClientContext &client;
                    std::shared_ptr<ClientMembershipListener> clientMembershipListener;
                    util::Sync<std::unordered_map<Address, std::shared_ptr<Member> > > members;
                    util::SynchronizedMap<std::string, MembershipListener> listeners;

                    std::mutex initialMembershipListenerMutex;

                    std::string addMembershipListenerWithoutInit(const std::shared_ptr<MembershipListener> &listener);

                    void initMembershipListener(MembershipListener &listener);

                    void fireMembershipEvent(const MembershipEvent &event);

                    void fireInitialMembershipEvent(const InitialMembershipEvent &event);
                };

            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
