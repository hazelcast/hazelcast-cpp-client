/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_CLIENTCLUSTERSERVICEIMPL_H
#define HAZELCAST_CLIENT_SPI_IMPL_CLIENTCLUSTERSERVICEIMPL_H

#include <map>

#include "hazelcast/client/spi/ClientClusterService.h"
#include "hazelcast/client/InitialMembershipListener.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/util/Atomic.h"
#include "hazelcast/util/SynchronizedMap.h"

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

                    virtual boost::shared_ptr<Member> getMember(const Address &address);

                    virtual boost::shared_ptr<Member> getMember(const std::string &uuid);

                    virtual std::vector<Member> getMemberList();

                    virtual boost::shared_ptr<Address> getMasterAddress();

                    virtual size_t getSize();

                    virtual std::string addMembershipListener(const boost::shared_ptr<MembershipListener> &listener);

                    virtual bool removeMembershipListener(const std::string &registrationId);

                    void handleMembershipEvent(const MembershipEvent &event);

                    void handleInitialMembershipEvent(const InitialMembershipEvent &event);

                    void listenMembershipEvents(const boost::shared_ptr<connection::Connection> &ownerConnection);

                    void fireMemberAttributeEvent(const MemberAttributeEvent &event);
                protected:
                    ClientContext &client;
                private:
                    boost::shared_ptr<ClientMembershipListener> clientMembershipListener;
                    util::Atomic<std::map<Address, boost::shared_ptr<Member> > > members;
                    util::SynchronizedValueMap<std::string, boost::shared_ptr<MembershipListener> > listeners;

                    util::Mutex initialMembershipListenerMutex;

                    std::string addMembershipListenerWithoutInit(const boost::shared_ptr<MembershipListener> &listener);

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

#endif 
