//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/Address.h"
#include "hazelcast/client/impl/GetPartitionsRequest.h"
#include "hazelcast/client/impl/PartitionsResponse.h"
#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/InvocationService.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            PartitionService::PartitionService(ClusterService &clusterService, InvocationService& invocationService, serialization::SerializationService &serializationService, spi::LifecycleService &lifecycleService)
            :partitionCount(0)
            , invocationService(invocationService)
            , clusterService(clusterService)
            , serializationService(serializationService)
            , lifecycleService(lifecycleService) {

            };

            PartitionService::~PartitionService() {
                std::vector<Address *> valArr = partitions.values();
                for (int i = 0; i < valArr.size(); i++) {
                    delete valArr[i];
                }

            };

            void PartitionService::start() {
                getInitialPartitions();
                boost::thread *partitionListener = new boost::thread(boost::bind(&PartitionService::runListener, this));
                partitionListenerThread.reset(partitionListener);
            };


            void PartitionService::stop() {
                partitionListenerThread->interrupt();
                partitionListenerThread->join();
            }

            Address *PartitionService::getPartitionOwner(int partitionId) {
                return partitions.get(partitionId);
            };

            int PartitionService::getPartitionId(const serialization::Data &key) {
                const int pc = partitionCount;
                int hash = key.getPartitionHash();
                return (hash == INT_MIN) ? 0 : abs(hash) % pc;
            };


            void PartitionService::runListener() {
                while (lifecycleService.isRunning()) {
                    try {
                        boost::this_thread::sleep(boost::posix_time::seconds(10));
                        if (!lifecycleService.isRunning()) {
                            break;
                        }
                        runRefresher();
                    } catch(boost::thread_interrupted &) {
                        break;
                    } catch(...) {
                        //ignored
                    }
                }
            };

            void PartitionService::runRefresher() {
                bool expected = false;
                if (updating.compare_exchange_strong(expected, true)) {
                    try {
                        boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                        std::auto_ptr<Address> ptr = clusterService.getMasterAddress();
                        if (ptr.get() == NULL) {
                            partitionResponse = getPartitionsFrom();
                        } else {
                            partitionResponse = getPartitionsFrom(*ptr.get());
                        }
                        if (partitionResponse != NULL) {
                            processPartitionResponse(*partitionResponse);
                        }
                    } catch(...) {
                        //ignored
                    }
                    updating = false;
                }

            };

            boost::shared_ptr<impl::PartitionsResponse> PartitionService::getPartitionsFrom(const Address &address) {
                impl::GetPartitionsRequest getPartitionsRequest;
                boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                try {
                    boost::shared_future<serialization::Data> future = invocationService.invokeOnTarget(getPartitionsRequest, address);
                    partitionResponse = serializationService.toObject<impl::PartitionsResponse>(future.get());
                } catch(exception::IOException &e) {
                    std::cerr << "Error while fetching cluster partition table " << e.what() << std::endl;
                }
                return partitionResponse;
            };


            boost::shared_ptr<impl::PartitionsResponse>PartitionService::getPartitionsFrom() {
                impl::GetPartitionsRequest getPartitionsRequest;
                boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                try {
                    boost::shared_future<serialization::Data> future = invocationService.invokeOnRandomTarget(getPartitionsRequest);
                    partitionResponse = serializationService.toObject<impl::PartitionsResponse>(future.get());
                } catch(exception::IOException &e) {
                    std::cerr << e.what() << std::endl;
                }
                return partitionResponse;
            }

            void PartitionService::processPartitionResponse(impl::PartitionsResponse &response) {
                const std::vector<Address> &members = response.getMembers();
                const std::vector<int> &ownerIndexes = response.getOwnerIndexes();
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
                std::vector<connection::Member> memberList = clusterService.getMemberList();
                for (std::vector<connection::Member>::iterator it = memberList.begin(); it < memberList.end(); ++it) {
                    Address target = (*it).getAddress();
                    boost::shared_ptr<impl::PartitionsResponse> response = getPartitionsFrom(target);
                    if (response != NULL) {
                        processPartitionResponse(*response);
                        return;
                    }
                }
                throw exception::IException("PartitionService::getInitialPartitions", " Cannot get initial partitions!");
            };


        }
    }
}