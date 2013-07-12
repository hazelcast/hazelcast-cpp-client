//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/Address.h"
#include "hazelcast/client/impl/GetPartitionsRequest.h"
#include "hazelcast/client/impl/PartitionsResponse.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClusterService.h"

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
                    impl::PartitionsResponse partitionResponse;
                    if (clusterService.isMemberListEmpty()) {
                        partitionResponse = getPartitionsFrom();
                    } else {
                        Address address = clusterService.getMasterAddress();
                        partitionResponse = getPartitionsFrom(address);
                    }
                    if (!partitionResponse.isEmpty()) {
                        processPartitionResponse(partitionResponse);
                    }
                }
            };


            void PartitionService::runRefresher() {
                impl::PartitionsResponse partitionResponse;
                if (clusterService.isMemberListEmpty()) {
                    partitionResponse = getPartitionsFrom();
                } else {
                    Address address = clusterService.getMasterAddress();
                    partitionResponse = getPartitionsFrom(address);

                }
                if (!partitionResponse.isEmpty()) {
                    processPartitionResponse(partitionResponse);
                }
            };

            impl::PartitionsResponse PartitionService::getPartitionsFrom(const Address  & address) {
                impl::GetPartitionsRequest getPartitionsRequest;
                impl::PartitionsResponse partitionResponse;
                try{
                    partitionResponse = clusterService.sendAndReceive<impl::PartitionsResponse>(address, getPartitionsRequest);
                }catch(exception::IOException& e){
                    std::cerr << e.what() << std::endl;
                }
                return partitionResponse;
            };


            impl::PartitionsResponse PartitionService::getPartitionsFrom() {
                impl::GetPartitionsRequest getPartitionsRequest;
                impl::PartitionsResponse partitionResponse;
                try{
                    partitionResponse = clusterService.sendAndReceive<impl::PartitionsResponse>(getPartitionsRequest);
                }catch(exception::IOException& e){
                    std::cerr << e.what() << std::endl;
                }
                return partitionResponse;
            }

            void PartitionService::processPartitionResponse(impl::PartitionsResponse & response) {
                vector<Address> members = response.getMembers();
                vector<int> ownerIndexes = response.getOwnerIndexes();
                if (partitionCount == 0) {
                    partitionCount = ownerIndexes.size();
                }
                for (int partitionId = 0; partitionId < partitionCount; ++partitionId) {
                    int ownerIndex = ownerIndexes[partitionId];
                    if (ownerIndex > -1) {
                        Address *address = new Address(members[ownerIndex]);
                        Address *pAddress = partitions.put(partitionId, address);
                        if (pAddress) delete pAddress;
                    }
                }
            };

            void PartitionService::getInitialPartitions() {
                vector<connection::Member> memberList = clusterService.getMemberList();
                for (vector<connection::Member>::iterator it = memberList.begin(); it < memberList.end(); ++it) {
                    Address target = (*it).getAddress();
                    impl::PartitionsResponse response = getPartitionsFrom(target);
                    if (!response.isEmpty()) {
                        processPartitionResponse(response);
                        return;
                    }
                }
                throw exception::IException("PartitionService::getInitialPartitions", " Cannot get initial partitions!");
            };


        }
    }
}