/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <hazelcast/client/hazelcast_client.h>
#include <hazelcast/client/membership_listener.h>
#include <hazelcast/client/initial_membership_event.h>
#include <hazelcast/client/membership_event.h>

hazelcast::client::membership_listener make_membership_listener() {
    return hazelcast::client::membership_listener()
            .on_joined([](const hazelcast::client::membership_event &membership_event) {
                std::cout << "New member joined: "
                          << membership_event.get_member().get_address() << std::endl;
            })
            .on_left([](const hazelcast::client::membership_event &membership_event) {
                std::cout << "Member left: "
                          << membership_event.get_member().get_address() << std::endl;
            });
}

hazelcast::client::membership_listener make_initial_membership_listener() {
    return hazelcast::client::membership_listener()
            .on_init([](const hazelcast::client::initial_membership_event &event) {
                auto members = event.get_members();
                std::cout << "The following are the initial members in the cluster:" << std::endl;
                for (const auto &member : members) {
                    std::cout << member.get_address() << std::endl;
                }
            })
            .on_joined([](const hazelcast::client::membership_event &membership_event) {
                std::cout << "New member joined: " <<
                          membership_event.get_member().get_address() << std::endl;
        })
        .on_left([](const hazelcast::client::membership_event &membership_event) {
            std::cout << "Member left: " <<
            membership_event.get_member().get_address() << std::endl;
        });
}

int main() {
    auto memberListener = make_membership_listener();
    auto initialMemberListener = make_initial_membership_listener();

    hazelcast::client::cluster *clusterPtr = nullptr;
    boost::uuids::uuid listenerId, initialListenerId;
    try {
        auto hz = hazelcast::new_client().get();

        hazelcast::client::cluster &cluster = hz.get_cluster();
        clusterPtr = &cluster;
        auto members = cluster.get_members();
        std::cout << "The following are members in the cluster:" << std::endl;
        for (const auto &member: members) {
            std::cout << member.get_address() << std::endl;
        }

        listenerId = cluster.add_membership_listener(std::move(memberListener));
        initialListenerId = cluster.add_membership_listener(std::move(initialMemberListener));

        // sleep some time for the events to be delivered before exiting
        std::this_thread::sleep_for(std::chrono::seconds(3));

        cluster.remove_membership_listener(listenerId);
        cluster.remove_membership_listener(initialListenerId);
    } catch (hazelcast::client::exception::iexception &e) {
        std::cerr << "Test failed !!! " << e.what() << std::endl;
        if (nullptr != clusterPtr) {
            clusterPtr->remove_membership_listener(listenerId);
            clusterPtr->remove_membership_listener(initialListenerId);
        }
        exit(-1);
    }

    std::cout << "Finished" << std::endl;

    return 0;
}
