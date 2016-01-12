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
// Created by İhsan Demir on 21/12/15.
//
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/MemberAttributeEvent.h>
#include <hazelcast/client/InitialMembershipListener.h>
#include <hazelcast/client/InitialMembershipEvent.h>

class MyInitialMemberListener : public hazelcast::client::InitialMembershipListener {

public:
    void init(const hazelcast::client::InitialMembershipEvent &event) {
        std::vector<hazelcast::client::Member> members = event.getMembers();
        std::cout << "The following are the initial members in the cluster:" << std::endl;
        for (std::vector<hazelcast::client::Member>::const_iterator it = members.begin(); it != members.end(); ++it) {
            std::cout << it->getAddress() << std::endl;
        }
    }

    void memberAdded(const hazelcast::client::MembershipEvent &membershipEvent) {
        std::cout << "[MyInitialMemberListener::memberAdded] New member joined:" <<
        membershipEvent.getMember().getAddress() <<
        std::endl;
    }

    void memberRemoved(const hazelcast::client::MembershipEvent &membershipEvent) {
        std::cout << "[MyInitialMemberListener::memberRemoved] Member left:" <<
        membershipEvent.getMember().getAddress() << std::endl;
    }

    void memberAttributeChanged(const hazelcast::client::MemberAttributeEvent &memberAttributeEvent) {
        std::cout << "[MyInitialMemberListener::memberAttributeChanged] Member attribute:" <<
        memberAttributeEvent.getKey()
        << " changed. Value:" << memberAttributeEvent.getValue() << " for member:" <<
        memberAttributeEvent.getMember().getAddress() << std::endl;
    }
};

class MyMemberListener : public hazelcast::client::MembershipListener {

public:
    void memberAdded(const hazelcast::client::MembershipEvent &membershipEvent) {
        std::cout << "[MyMemberListener::memberAdded] New member joined:" << membershipEvent.getMember().getAddress() <<
        std::endl;
    }

    void memberRemoved(const hazelcast::client::MembershipEvent &membershipEvent) {
        std::cout << "[MyMemberListener::memberRemoved] Member left:" <<
        membershipEvent.getMember().getAddress() << std::endl;
    }

    void memberAttributeChanged(const hazelcast::client::MemberAttributeEvent &memberAttributeEvent) {
        std::cout << "[MyMemberListener::memberAttributeChanged] Member attribute:" << memberAttributeEvent.getKey()
        << " changed. Value:" << memberAttributeEvent.getValue() << " for member:" <<
        memberAttributeEvent.getMember().getAddress() << std::endl;
    }
};

int main() {
    MyMemberListener memberListener;
    MyInitialMemberListener initialMemberListener;

    hazelcast::client::Cluster *clusterPtr = NULL;
    try {
        hazelcast::client::ClientConfig config;
        hazelcast::client::HazelcastClient hz(config);

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
        hazelcast::util::sleep(3);

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
