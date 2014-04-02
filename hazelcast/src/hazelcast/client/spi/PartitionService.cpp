//
// Created by sancar koyunlu on 6/3/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/PartitionService.h"
#include "hazelcast/client/spi/ClusterService.h"
#include "hazelcast/client/spi/LifecycleService.h"
#include "hazelcast/client/spi/InvocationService.h"
#include "hazelcast/client/impl/GetPartitionsRequest.h"
#include "hazelcast/client/impl/PartitionsResponse.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/util/LockGuard.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            PartitionService::PartitionService(spi::ClientContext &clientContext)
            : clientContext(clientContext)
            , updating(false)
            , partitionCount(0) {

            };

            bool PartitionService::start() {
                util::LockGuard lg(startLock);
                if(!getInitialPartitions()){
                    return false;
                }
                util::Thread *partitionListener = new util::Thread(PartitionService::staticRunListener, this);
                partitionListenerThread.reset(partitionListener);
                return true;
            }

            void PartitionService::stop() {
                util::LockGuard lg(startLock);
                if (partitionListenerThread.get() != NULL) {
                    partitionListenerThread->interrupt();
                    partitionListenerThread->join();
                }
            }

            boost::shared_ptr<Address> PartitionService::getPartitionOwner(int partitionId) {
                return partitions.get(partitionId);
            }

            int PartitionService::getPartitionId(const serialization::pimpl::Data &key) {
                const int pc = partitionCount;
                int hash = key.getPartitionHash();
                return (hash == INT_MIN) ? 0 : abs(hash) % pc;
            };

            void PartitionService::staticRunListener(util::ThreadArgs& args){
                PartitionService* partitionService = (PartitionService*)args.arg0;
                partitionService->runListener();
            }

            void PartitionService::runListener() {
                while (clientContext.getLifecycleService().isRunning()) {
                    try {
                        sleep(10);
                        if (!clientContext.getLifecycleService().isRunning()) {
                            break;
                        }
                        runRefresher();
                    } catch(exception::IException &e) {
                        util::ILogger::getLogger().warning(std::string("PartitionService::runListener") + e.what());
                    } catch(...) {
                        util::ILogger::getLogger().severe("PartitionService::runListener unkown exception");
                    }
                }
            };

            void PartitionService::runRefresher() {

                if (updating.compareAndSet(false, true)) {
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
                    boost::shared_ptr< util::Future<serialization::pimpl::Data> >  future = clientContext.getInvocationService().invokeOnTarget(request, address);
                    partitionResponse = clientContext.getSerializationService().toObject<impl::PartitionsResponse>(future->get());
                } catch(exception::IOException &e) {
                    util::ILogger::getLogger().severe(std::string("Error while fetching cluster partition table => ") + e.what());
                }
                return partitionResponse;
            };

            boost::shared_ptr<impl::PartitionsResponse>PartitionService::getPartitionsFrom() {
                impl::GetPartitionsRequest *request = new impl::GetPartitionsRequest();
                boost::shared_ptr<impl::PartitionsResponse> partitionResponse;
                try {
                    boost::shared_ptr< util::Future<serialization::pimpl::Data> >  future = clientContext.getInvocationService().invokeOnRandomTarget(request);
                    partitionResponse = clientContext.getSerializationService().toObject<impl::PartitionsResponse>(future->get());
                } catch(exception::IOException &e) {
                    util::ILogger::getLogger().warning(std::string("Error while fetching cluster partition table => ") + e.what());
                }
                return partitionResponse;
            }

            void PartitionService::processPartitionResponse(impl::PartitionsResponse &response) {
                const std::vector<Address> &members = response.getMembers();
                const std::vector<int> &ownerIndexes = response.getOwnerIndexes();
                if (partitionCount == 0) {
                    partitionCount = ownerIndexes.size();
                }
                for (int partitionId = 0; partitionId < (int)partitionCount; ++partitionId) {
                    int ownerIndex = ownerIndexes[partitionId];
                    if (ownerIndex > -1) {
                        boost::shared_ptr<Address> address(new Address(members[ownerIndex]));
                        partitions.put(partitionId, address);
                    }
                }
            };

            bool PartitionService::getInitialPartitions() {
                std::vector<Member> memberList = clientContext.getClusterService().getMemberList();
                for (std::vector<Member>::iterator it = memberList.begin(); it < memberList.end(); ++it) {
                    Address target = (*it).getAddress();
                    boost::shared_ptr<impl::PartitionsResponse> response = getPartitionsFrom(target);
                    if (response != NULL) {
                        processPartitionResponse(*response);
                        return true;
                    }
                }
                util::ILogger::getLogger().severe("PartitionService::getInitialPartitions Cannot get initial partitions!");
                return false;
            };


        }
    }
}