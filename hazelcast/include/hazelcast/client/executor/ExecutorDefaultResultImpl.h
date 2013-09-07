//
// Created by sancar koyunlu on 9/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_ExecutorDefaultResultImpl
#define HAZELCAST_ExecutorDefaultResultImpl

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
            class ExecutorDefaultResultImpl {
            public:
                ExecutorDefaultResultImpl() {

                }

                void init(std::string *instanceName, spi::ClientContext *clientContext) {
                    this->instanceName = instanceName;
                    this->context = clientContext;
                }

                template<typename Result, typename Callable >
                Future<Result> submit(Callable& task, const Result& result) {
                    executor::LocalTargetCallableRequest<Callable> request(*instanceName, task);
                    Future<Result> future;
                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultResultImpl::asyncInvoke<Result, Callable>, this, boost::cref(request), boost::ref(future), boost::cref(result)));
                    return future;
                }

                template<typename Result, typename Callable >
                Future<Result> submit(Callable& task, const serialization::Data& partitionKey, const Result& result) {
                    spi::PartitionService & partitionService = context->getPartitionService();
                    int partitionId = partitionService.getPartitionId(partitionKey);
                    Address *pointer = partitionService.getPartitionOwner(partitionId);
                    return submit(task, *pointer, result);
                }

                template<typename Result, typename Callable >
                Future<Result> submit(Callable& task, const Address& address, const Result& result) {
                    executor::TargetCallableRequest<Callable> request(*instanceName, task, address);
                    Future<Result> future;
                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultResultImpl::asyncInvokeToAddress, this, boost::cref(request), boost::cref(address), boost::ref(future), boost::cref(result)));
                    return future;
                }


            private:
                template<typename Result, typename Callable>
                void asyncInvokeToAddress(const executor::TargetCallableRequest<Callable> & request, const Address& address, Future<Result>& future, const Result& result) {
                    try{
                        invoke<bool>(request, address);
                        future.accessInternal().setValue((new Result(result)));
                    } catch(exception::ServerException& ex){
                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
                    } catch(...){
                        std::cerr << "Exception in IExecuterService::asyncInvokeWithDefaultResult" << std::endl;
                    }
                }

                template<typename Result, typename Callable>
                void asyncInvoke(const executor::LocalTargetCallableRequest<Callable> & request, Future<Result>& future, const Result& result) {
                    try{
                        invoke<bool>(request);
                        future.accessInternal().setValue(new Result(result));
                    } catch(exception::ServerException& ex){
                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
                    } catch(...){
                        std::cerr << "Exception in IExecuterService::asyncInvokeWithDefaultResult" << std::endl;
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


#endif //HAZELCAST_ExecutorDefaultResultImpl
