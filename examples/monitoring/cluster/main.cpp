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
#include <hazelcast/client/InitialMembershipListener.h>
#include <hazelcast/client/InitialMembershipEvent.h>
#include <hazelcast/client/MembershipEvent.h>

class MyInitialMemberListener : public hazelcast::client::InitialMembershipListener {

public:
    void init(hazelcast::client::InitialMembershipEvent event) override {
        auto members = event.getMembers();
        std::cout << "The following are the initial members in the cluster:" << std::endl;
        for (const auto &member : members) {
            std::cout << member.getAddress() << std::endl;
        }
    }

    void memberAdded(const hazelcast::client::MembershipEvent &membershipEvent) override {
        std::cout << "[MyInitialMemberListener::memberAdded] New member joined:" <<
        membershipEvent.getMember().getAddress() << std::endl;
    }

    void memberRemoved(const hazelcast::client::MembershipEvent &membershipEvent) override {
        std::cout << "[MyInitialMemberListener::memberRemoved] Member left:" <<
        membershipEvent.getMember().getAddress() << std::endl;
    }

};

class MyMemberListener : public hazelcast::client::MembershipListener {

public:
    void memberAdded(const hazelcast::client::MembershipEvent &membershipEvent) override {
        std::cout << "[MyMemberListener::memberAdded] New member joined:" << membershipEvent.getMember().getAddress() <<
        std::endl;
    }

    void memberRemoved(const hazelcast::client::MembershipEvent &membershipEvent) override {
        std::cout << "[MyMemberListener::memberRemoved] Member left:" <<
        membershipEvent.getMember().getAddress() << std::endl;
    }
};

int main() {
    MyMemberListener memberListener;
    MyInitialMemberListener initialMemberListener;

    hazelcast::client::Cluster *clusterPtr = nullptr;
    try {
        hazelcast::client::HazelcastClient hz;

        hazelcast::client::Cluster &cluster = hz.getCluster();
        clusterPtr = &cluster;
        std::vector<hazelcast::client::Member> members = cluster.getMembers();
        std::cout << "The following are members in the cluster:" << std::endl;
        for (std::vector<hazelcast::client::Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
            std::cout << it->getAddress() << std::endl;
        }

        cluster.addMembershipListener(&memberListener);

        cluster.addMembershipListener(&initialMemberListener);

        // sleep some time for the events to be delivered before exiting
        std::this_thread::sleep_for(std::chrono::seconds(3));

        cluster.removeMembershipListener(&memberListener);
        cluster.removeMembershipListener(&initialMemberListener);
    } catch (hazelcast::client::exception::IException &e) {
        std::cerr << "Test failed !!! " << e.what() << std::endl;
        if (NULL != clusterPtr) {
            clusterPtr->removeMembershipListener(&memberListener);
            clusterPtr->removeMembershipListener(&initialMemberListener);
        }
        exit(-1);
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
