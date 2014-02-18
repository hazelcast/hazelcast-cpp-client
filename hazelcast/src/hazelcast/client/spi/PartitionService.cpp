//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/GetPartitionsRequest.h"
#include "hazelcast/client/impl/PartitionsResponse.h"
#include "hazelcast/client/serialization/SerializationService.h"
#include "hazelcast/client/spi/ClientContext.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            PartitionService::PartitionService(spi::ClientContext &clientContext)
            :partitionCount(0)
            , updating(false)
            , clientContext(clientContext) {

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

            boost::shared_ptr<Address> PartitionService::getPartitionOwner(int partitionId) {
                return partitions.get(partitionId);
            }

            int PartitionService::getPartitionId(const serialization::Data &key) {
                const int pc = partitionCount;
                int hash = key.getPartitionHash();
                return (hash == INT_MIN) ? 0 : abs(hash) % pc;
            };

            void PartitionService::runListener() {
                while (clientContext.getLifecycleService().isRunning()) {
                    try {
                        boost::this_thread::sleep(boost::posix_time::seconds(10));
                        if (!clientContext.getLifecycleService().isRunning()) {
                            break;
                        }
                        runRefresher();
                    } catch(boost::thread_interrupted &) {
                        break;
                    } catch(...) {
                        //ignored
                        std::cerr << "PartitionService::runListener ignored exception " << std::endl;
                    }
                }
            };

            void PartitionService::runRefresher() {
                bool expected = false;
                if (updating.compare_exchange_strong(expected, true)) {
                    try {
                        boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                        std::auto_ptr<Address> ptr = clientContext.getClusterService().getMasterAddress();
                        if (ptr.get() == NULL) {
                            partitionResponse = getPartitionsFrom();
                        } else {
                            partitionResponse = getPartitionsFrom(*ptr.get());
                        }
                        if (partitionResponse != NULL) {
                            processPartitionResponse(*partitionResponse);
                        }
                    } catch(...) {
                    }
                    updating = false;
                }

            };

            boost::shared_ptr<impl::PartitionsResponse> PartitionService::getPartitionsFrom(const Address &address) {
                impl::GetPartitionsRequest *request = new impl::GetPartitionsRequest();
                boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                try {
                    boost::shared_future<serialization::Data> future = clientContext.getInvocationService().invokeOnTarget(request, address);
                    partitionResponse = clientContext.getSerializationService().toObject<impl::PartitionsResponse>(future.get());
                } catch(exception::IOException &e) {
                    std::cerr << "Error while fetching cluster partition table " << e.what() << std::endl;
                }
                return partitionResponse;
            };


            boost::shared_ptr<impl::PartitionsResponse>PartitionService::getPartitionsFrom() {
                impl::GetPartitionsRequest *request = new impl::GetPartitionsRequest();
                boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                try {
                    boost::shared_future<serialization::Data> future = clientContext.getInvocationService().invokeOnRandomTarget(request);
                    partitionResponse = clientContext.getSerializationService().toObject<impl::PartitionsResponse>(future.get());
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
                        boost::shared_ptr<Address> address(new Address(members[ownerIndex]));
                        partitions.put(partitionId, address);
                    }
                }
            };

            void PartitionService::getInitialPartitions() {
                std::vector<Member> memberList = clientContext.getClusterService().getMemberList();
                for (std::vector<Member>::iterator it = memberList.begin(); it < memberList.end(); ++it) {
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