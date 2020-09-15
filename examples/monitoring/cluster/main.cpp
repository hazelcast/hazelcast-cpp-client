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
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/MembershipListener.h>
#include <hazelcast/client/InitialMembershipEvent.h>
#include <hazelcast/client/MembershipEvent.h>


MembershipListener makeMembershipListener() {
    return MembershipListener()
        .on_joined([](const hazelcast::client::MembershipEvent &membershipEvent) {
            std::cout << "New member joined: "
                << membershipEvent.getMember().getAddress() << std::endl;
        })
        .on_left([](const hazelcast::client::MembershipEvent &membershipEvent) {
            std::cout << "Member left: " 
                << membershipEvent.getMember().getAddress() << std::endl;
        });
}

MembershipListener makeInitialMembershipListener() {
    return MembershipListener()
        .on_init([](const hazelcast::client::InitialMembershipEvent &event){
            auto members = event.getMembers();
            std::cout << "The following are the initial members in the cluster:" << std::endl;
            for (const auto &member : members) {
                std::cout << member.getAddress() << std::endl;
            }
        })
        .on_joined([](const hazelcast::client::MembershipEvent &membershipEvent) {
            std::cout << "New member joined: " <<
            membershipEvent.getMember().getAddress() << std::endl;
        })
        .on_left([](const hazelcast::client::MembershipEvent &membershipEvent) {
            std::cout << "Member left: " <<
            membershipEvent.getMember().getAddress() << std::endl;
        });
}

int main() {
    auto memberListener = makeMembershipListener();
    auto initialMemberListener = makeInitialMembershipListener();

    hazelcast::client::Cluster *clusterPtr = nullptr;
    boost::uuids::uuid listenerId, initialListenerId;
    try {
        hazelcast::client::HazelcastClient hz;

        hazelcast::client::Cluster &cluster = hz.getCluster();
        clusterPtr = &cluster;
        auto members = cluster.getMembers();
        std::cout << "The following are members in the cluster:" << std::endl;
        for (const auto &member: members) {
            std::cout << member.getAddress() << std::endl;
        }

        listenerId = cluster.addMembershipListener(std::move(memberListener));
        initialListenerId = cluster.addMembershipListener(std::move(initialMemberListener));

        // sleep some time for the events to be delivered before exiting
        std::this_thread::sleep_for(std::chrono::seconds(3));

        cluster.removeMembershipListener(listenerId);
        cluster.removeMembershipListener(initialListenerId);
    } catch (hazelcast::client::exception::IException &e) {
        std::cerr << "Test failed !!! " << e.what() << std::endl;
        if (nullptr != clusterPtr) {
            clusterPtr->removeMembershipListener(listenerId);
            clusterPtr->removeMembershipListener(initialListenerId);
        }
        exit(-1);
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
