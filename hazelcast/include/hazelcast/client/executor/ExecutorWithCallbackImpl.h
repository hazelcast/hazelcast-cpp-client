////
//// Created by sancar koyunlu on 9/7/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//
//#ifndef HAZELCAST_ExecutorWithCallbackImpl
//#define HAZELCAST_ExecutorWithCallbackImpl
//
//#include "hazelcast/client/spi/ClientContext.h"
//#include "hazelcast/client/serialization/pimpl/Data.h"
//#include "hazelcast/client/Address.h"
//#include "hazelcast/client/spi/InvocationService.h"
//#include "hazelcast/client/executor/LocalTargetCallableRequest.h"
//#include "hazelcast/client/executor/TargetCallableRequest.h"
//#include "hazelcast/client/impl/MultiExecutionCallbackWrapper.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace executor {
//            class ExecutorWithCallbackImpl {
//            public:
//                ExecutorWithCallbackImpl(const std::string &instanceName, spi::ClientContext *context)
//                :instanceName(instanceName), context(context) {
//
//                }
//
//                template<typename Result, typename Callable, typename ExecutionCallback>
//                void submit(Callable &task, ExecutionCallback &callback) {
//                    boost::thread asyncInvokeThread(boost::bind(&ExecutorWithCallbackImpl::asyncInvoke<Result, Callable, ExecutionCallback>, this, boost::ref(task), boost::ref(callback)));
//                }
//
//                template<typename Result, typename Callable, typename ExecutionCallback>
//                void submit(Callable &task, const serialization::pimpl::Data &partitionKey, ExecutionCallback &callback) {
//                    spi::PartitionService &partitionService = context->getPartitionService();
//                    int partitionId = partitionService.getPartitionId(partitionKey);
//                    Address *pointer = partitionService.getPartitionOwner(partitionId);
//                    if (pointer != NULL)
//                        submit<Result>(task, *pointer, callback);
//                    else
//                        return submit<Result>(task, callback);
//                }
//
//                template<typename Result, typename Callable, typename ExecutionCallback>
//                void submit(Callable &task, const Address &address, ExecutionCallback &callback) {
//                    boost::thread asyncInvokeThread(boost::bind(&ExecutorWithCallbackImpl::asyncInvokeToAddress<Result, Callable, ExecutionCallback>, this, boost::ref(task), address, boost::ref(callback)));
//                }
//
//                template<typename Result, typename Callable, typename MultiExecutionCallback>
//                void submitMulti(Callable &task, const Member &member, util::AtomicPointer<impl::MultiExecutionCallbackWrapper<Result, MultiExecutionCallback > > callback) {
//                    Address const &address = member.getAddress();
//                    if (context->getClusterService().isMemberExists(address)) {
//                        boost::thread asyncInvokeThread(boost::bind(&ExecutorWithCallbackImpl::asyncInvokeWithMultiCallback<Result, Callable, MultiExecutionCallback >, this, boost::ref(task), member, callback));
//                    } else {
//                        throw exception::IException("IExecuterService::executeOnMember()", "Member is not available!!!");
//                    }
//                };
//            private:
//                template<typename Result, typename Callable, typename ExecutionCallback>
//                void asyncInvokeToAddress(Callable &task, Address address, ExecutionCallback &callback) {
//                    executor::TargetCallableRequest<Callable> request(instanceName, task, address);
//                    try {
//                        boost::shared_ptr<Result> result = invoke<Result>(request, address);
//                        callback.onResponse(*result);
//                    } catch(std::exception &e) {
//                        callback.onFailure(e);
//                    }
//                }
//
//                template<typename Result, typename Callable, typename ExecutionCallback>
//                void asyncInvoke(Callable &task, ExecutionCallback &callback) {
//                    executor::LocalTargetCallableRequest<Callable> request(instanceName, task);
//                    try {
//                        boost::shared_ptr<Result> result = invoke<Result>(request);
//                        callback.onResponse(*result);
//                    } catch(std::exception &e) {
//                        callback.onFailure(e);
//                    }
//                }
//
//
//                template<typename Result, typename Callable, typename MultiExecutionCallback>
//                void asyncInvokeWithMultiCallback(Callable &task, Member member, util::AtomicPointer<impl::MultiExecutionCallbackWrapper<Result, MultiExecutionCallback > > callback) {
//                    Address address = member.getAddress();
//                    executor::TargetCallableRequest<Callable> request(instanceName, task, address);
//                    try {
//                        boost::shared_ptr<Result> result = invoke<Result>(request, address);
//                        callback->onResponse(member, *result);
//                    } catch(std::exception &) {
//                    }
//                }
//
//                template<typename Result, typename Request>
//                boost::shared_ptr<Result> invoke(const Request &request, const Address &target) {
//                    return context->getInvocationService().invokeOnTarget<Result>(request, target);
//                }
//
//                template<typename Result, typename Request>
//                boost::shared_ptr<Result> invoke(const Request &request) {
//                    return context->getInvocationService().invokeOnRandomTarget<Result>(request);
//                }
//
//                template<typename T>
//                serialization::pimpl::Data toData(const T &o) {
//                    return context->getSerializationService().toData<T>(&o);
//                }
//
//                const std::string &instanceName;
//                spi::ClientContext *context;
//            };
//        }
//    }
//}
//
//#endif //HAZELCAST_ExecutorWithCallbackImpl
