//
// Created by sancar koyunlu on 9/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ExecutorDefaultImpl
#define HAZELCAST_ExecutorDefaultImpl

#include "ClientContext.h"
#include "Data.h"
#include "Address.h"
#include "InvocationService.h"
#include "Future.h"
#include "LocalTargetCallableRequest.h"
#include "TargetCallableRequest.h"

namespace hazelcast {
    namespace client {
        namespace executor {

            class ExecutorDefaultImpl {
            public:
                ExecutorDefaultImpl() {

                }

                void init(std::string *instanceName, spi::ClientContext *clientContext) {
                    this->instanceName = instanceName;
                    this->context = clientContext;
                }

                template<typename Result, typename Callable >
                Future<Result> submit(Callable& task) {
                    executor::LocalTargetCallableRequest<Callable> request(*instanceName, task);
                    Future<Result> future;
                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultImpl::asyncInvoke<Result, Callable>, this, boost::cref(request), boost::ref(future)));
                    return future;
                }

                template<typename Result, typename Callable >
                Future<Result> submit(Callable& task, const serialization::Data& partitionKey) {
                    spi::PartitionService & partitionService = context->getPartitionService();
                    int partitionId = partitionService.getPartitionId(partitionKey);
                    Address *pointer = partitionService.getPartitionOwner(partitionId);
                    return submit<Result>(task, *pointer);
                }

                template<typename Result, typename Callable >
                Future<Result> submit(Callable& task, const Address& address) {
                    Address lAddress = address;
                    executor::TargetCallableRequest<Callable> request(*instanceName, task, lAddress);
                    Future<Result> future;
                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultImpl::asyncInvokeToAddress<Result, Callable>, this, boost::cref(request), boost::cref(address), boost::ref(future)));
                    return future;
                }


            private:

                template<typename Result, typename Callable>
                void asyncInvokeToAddress(const executor::TargetCallableRequest<Callable> & request, const Address& address, Future<Result>& future) {
                    try{
                        future.accessInternal().setValue(new Result(invoke<Result>(request, address)));
                    } catch(exception::ServerException& ex){
                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
                    } catch(...){
                        std::cerr << "Exception in IExecuterService::asyncInvoke" << std::endl;
                    }
                }

                template<typename Result, typename Callable>
                void asyncInvoke(const executor::LocalTargetCallableRequest<Callable> & request, Future<Result>& future) {
                    try{
                        future.accessInternal().setValue(new Result(invoke<Result>(request)));
                    } catch(exception::ServerException& ex){
                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
                    } catch(...){
                        std::cerr << "Exception in IExecuterService::asyncInvoke" << std::endl;
                    }
                }


                template<typename Result, typename Request>
                Result invoke(const Request& request, const Address& target) {
                    return context->getInvocationService().invokeOnTarget<Result>(request, target);
                }

                template<typename Result, typename Request>
                Result invoke(const Request& request) {
                    return context->getInvocationService().invokeOnRandomTarget<Result>(request);
                }

                template<typename T>
                serialization::Data toData(const T& o) {
                    return context->getSerializationService().toData<T>(&o);
                }

                std::string *instanceName;
                spi::ClientContext *context;

            };

        }
    }
}

#endif //HAZELCAST_ExecutorDefaultImpl
