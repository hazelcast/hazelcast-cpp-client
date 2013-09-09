//
// Created by sancar koyunlu on 9/7/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ExecutorWithCallbackImpl
#define HAZELCAST_ExecutorWithCallbackImpl

#include "ClientContext.h"
#include "Data.h"
#include "Address.h"
#include "InvocationService.h"
#include "LocalTargetCallableRequest.h"
#include "TargetCallableRequest.h"
#include "MultiExecutionCallbackWrapper.h"

namespace hazelcast {
    namespace client {
        namespace executor {
            class ExecutorWithCallbackImpl {
            public:

                ExecutorWithCallbackImpl() {

                }

                void init(std::string *instanceName, spi::ClientContext *clientContext) {
                    this->instanceName = instanceName;
                    this->context = clientContext;
                }

                template<typename Result, typename Callable, typename ExecutionCallback>
                void submit(Callable& task, ExecutionCallback& callback) {
                    boost::thread asyncInvokeThread(boost::bind(&ExecutorWithCallbackImpl::asyncInvoke<Result, Callable, ExecutionCallback>, this, boost::ref(task), boost::ref(callback)));
                }

                template<typename Result, typename Callable, typename ExecutionCallback>
                void submit(Callable& task, const serialization::Data& partitionKey, ExecutionCallback& callback) {
                    spi::PartitionService & partitionService = context->getPartitionService();
                    int partitionId = partitionService.getPartitionId(partitionKey);
                    Address *pointer = partitionService.getPartitionOwner(partitionId);
                    if (pointer != NULL)
                        submit<Result>(task, *pointer, callback);
                    else
                        return submit<Result>(task, callback);
                }

                template<typename Result, typename Callable, typename ExecutionCallback>
                void submit(Callable& task, const Address& address, ExecutionCallback& callback) {
                    boost::thread asyncInvokeThread(boost::bind(&ExecutorWithCallbackImpl::asyncInvokeToAddress<Result, Callable, ExecutionCallback>, this, boost::ref(task), address, boost::ref(callback)));
                }

                template<typename Result, typename Callable, typename MultiExecutionCallback>
//                void submitMulti(Callable& task, const connection::Member& member, impl::MultiExecutionCallbackWrapper<Result, MultiExecutionCallback >& callback) {
                void submitMulti(Callable& task, const connection::Member& member, util::AtomicPointer<impl::MultiExecutionCallbackWrapper<Result, MultiExecutionCallback > > callback) {
                    Address const & address = member.getAddress();
                    if (context->getClusterService().isMemberExists(address)) {
                        boost::thread asyncInvokeThread(boost::bind(&ExecutorWithCallbackImpl::asyncInvokeWithMultiCallback<Result, Callable, MultiExecutionCallback >, this, boost::ref(task), member, callback));
                    } else {
                        throw exception::IException("IExecuterService::executeOnMember()", "Member is not available!!!");
                    }
                };
            private:
                template<typename Result, typename Callable, typename ExecutionCallback>
                void asyncInvokeToAddress(Callable & task, Address address, ExecutionCallback& callback) {
                    executor::TargetCallableRequest<Callable> request(*instanceName, task, address);
                    try{
                        Result result = invoke<Result>(request, address);
                        callback.onResponse(result);
                    } catch(std::exception& e){
                        callback.onFailure(e);
                    }
                }

                template<typename Result, typename Callable, typename ExecutionCallback>
                void asyncInvoke(Callable& task, ExecutionCallback& callback) {
                    executor::LocalTargetCallableRequest<Callable> request(*instanceName, task);
                    try{
                        Result result = invoke<Result>(request);
                        callback.onResponse(result);
                    } catch(std::exception& e){
                        callback.onFailure(e);
                    }
                }


                template<typename Result, typename Callable, typename MultiExecutionCallback>
//                void asyncInvokeWithMultiCallback(Callable& task, connection::Member member, impl::MultiExecutionCallbackWrapper<Result, MultiExecutionCallback >& callback) {
                void asyncInvokeWithMultiCallback(Callable& task, connection::Member member, util::AtomicPointer<impl::MultiExecutionCallbackWrapper<Result, MultiExecutionCallback > > callback) {
                    Address address = member.getAddress();
                    executor::TargetCallableRequest<Callable> request(*instanceName, task, address);
                    try{
                        Result result = invoke<Result>(request, address);
                        callback->onResponse(member, result);
                    } catch(std::exception& e){
                        //TODO ignored why?
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

#endif //HAZELCAST_ExecutorWithCallbackImpl
