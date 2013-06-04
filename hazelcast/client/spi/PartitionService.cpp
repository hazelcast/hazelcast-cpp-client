//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "../impl/GetPartitionsRequest.h"
#include "../impl/PartitionsResponse.h"
#include "PartitionService.h"
#include "ClusterService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            PartitionService::PartitionService(ClusterService & clusterService, serialization::SerializationService & serializationService)
            :partitionCount(0)
            , clusterService(clusterService)
            , serializationService(serializationService)
            , partitionListenerThread(PartitionService::startListener, this)
            , refreshPartitionThread(PartitionService::startRefresher, this) {

            };


            void PartitionService::start() {
                getInitialPartitions();
                std::cerr << "partitionCount = " << partitionCount << std::endl;
                partitionListenerThread.start();
            };


            void PartitionService::refreshPartitions() {
                refreshPartitionThread.start();
            };

            Address *PartitionService::getPartitionOwner(int partitionId) {
                return partitions.get(partitionId);
            };

            int PartitionService::getPartitionId(serialization::Data & key) {
                const int pc = partitionCount;
                int hash = key.getPartitionHash();
                return (hash == INT_MIN) ? 0 : abs(hash) % pc;
            };

            void *PartitionService::startListener(void *parameteres) {
                static_cast<PartitionService *>(parameteres)->runListener();
                return NULL;
            };

            void *PartitionService::startRefresher(void *parameteres) {
                static_cast<PartitionService *>(parameteres)->runRefresher();
                return NULL;
            };

            void PartitionService::runListener() {
                while (true) {
                    sleep(10);
                    impl::PartitionsResponse partitionResponse = getPartitionsFrom(clusterService.getMasterAddress());
                    if (!partitionResponse.isEmpty()) {
                        processPartitionResponse(partitionResponse);
                    }
                }
            };


            void PartitionService::runRefresher() {
                impl::PartitionsResponse partitionResponse = getPartitionsFrom(clusterService.getMasterAddress());
                if (!partitionResponse.isEmpty()) {
                    processPartitionResponse(partitionResponse);
                }
            };

            hazelcast::client::impl::PartitionsResponse PartitionService::getPartitionsFrom(Address const & address) {
                hazelcast::client::impl::PartitionsResponse partitionResponse;
                hazelcast::client::impl::GetPartitionsRequest getPartitionsRequest;
                clusterService.sendAndReceive(address, getPartitionsRequest, partitionResponse);
                return partitionResponse;
            };

            void PartitionService::processPartitionResponse(hazelcast::client::impl::PartitionsResponse & response) {
                vector<Address> members = response.getMembers();
                vector<int> ownerIndexes = response.getOwnerIndexes();
                if (partitionCount == 0) {
                    partitionCount = ownerIndexes.size();
                }
                for (int partitionId = 0; partitionId < partitionCount; ++partitionId) {
                    int ownerIndex = ownerIndexes[partitionId];
                    if (ownerIndex > -1) {
                        Address *address = new Address(members[ownerIndex]);
                        partitions.put(partitionId, address);
                    }
                }
            };

            void PartitionService::getInitialPartitions() {
                vector<connection::Member> memberList = clusterService.getMemberList();
                for (vector<connection::Member>::iterator it = memberList.begin(); it < memberList.end(); ++it) {
                    Address target = (*it).getAddress();
                    hazelcast::client::impl::PartitionsResponse response = getPartitionsFrom(target);
                    if (!response.isEmpty()) {
                        processPartitionResponse(response);
                        return;
                    }
                }
                throw HazelcastException("IllegalStateException :: Cannot get initial partitions!");
            };


        }
    }
}