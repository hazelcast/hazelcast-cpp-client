////
//// Created by sancar koyunlu on 9/7/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//
//
//#ifndef HAZELCAST_ExecutorDefaultResultImpl
//#define HAZELCAST_ExecutorDefaultResultImpl
//
//#include "hazelcast/client/spi/ClientContext.h"
//#include "hazelcast/client/serialization/pimpl/Data.h"
//#include "hazelcast/client/Address.h"
//#include "hazelcast/client/spi/InvocationService.h"
//#include "hazelcast/client/Future.h"
//#include "hazelcast/client/executor/LocalTargetCallableRequest.h"
//#include "hazelcast/client/executor/TargetCallableRequest.h"
//
//namespace hazelcast {
//    namespace client {
//        namespace executor {
//            class ExecutorDefaultResultImpl {
//            public:
//                ExecutorDefaultResultImpl(const std::string &instanceName, spi::ClientContext *context)
//                :instanceName(instanceName), context(context) {
//
//                }
//
//                template<typename Result, typename Callable >
//                Future<Result> submit(Callable &task, const Result &result) {
//                    Future<Result> future;
//                    util::Thread asyncInvokeThread(&ExecutorDefaultResultImpl::asyncInvoke<Result, Callable>, this, (task), future, (result)));
//                    return future;
//                }
//
//                template<typename Result, typename Runnable >
//                Future<Result> submit(executor::RunnableAdapter<Runnable> task, const Result &result) {
//                    Future<Result> future;
//                    util::Thread asyncInvokeThread(&ExecutorDefaultResultImpl::asyncInvoke<Result, executor::RunnableAdapter<Runnable> >, this, task, future, (result)));
//                    return future;
//                }
//
//                template<typename Result, typename Callable >
//                Future<Result> submit(Callable &task, const serialization::pimpl::Data &partitionKey, const Result &result) {
//                    spi::PartitionService &partitionService = context->getPartitionService();
//                    int partitionId = partitionService.getPartitionId(partitionKey);
//                    Address *pointer = partitionService.getPartitionOwner(partitionId);
//                    if (pointer != NULL)
//                        return submit(task, *pointer, result);
//                    else
//                        return submit(task, result);
//                }
//
//                template<typename Result, typename Callable >
//                Future<Result> submit(Callable &task, const Address &address, const Result &result) {
//                    Future<Result> future;
//                    util::Thread asyncInvokeThread(&ExecutorDefaultResultImpl::asyncInvokeToAddress, this, (task), address, future, (result)));
//                    return future;
//                }
//
//
//            private:
//                template<typename Result, typename Callable>
//                void asyncInvokeToAddress(Callable &task, Address address, Future<Result> future, const Result &result) {
//                    executor::TargetCallableRequest<Callable> request(instanceName, task, address);
//                    try{
//                        invoke<serialization::pimpl::Void>(request, address);
//                        future.accessInternal().setValue((new Result(result)));
//                    } catch(exception::ServerException &ex){
//                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
//                    } catch(...){
//                        logger << "Exception in IExecuterService::asyncInvokeWithDefaultResult" << std::endl;
//                    }
//                }
//
//                template<typename Result, typename Callable>
//                void asyncInvoke(Callable &callable, Future<Result> future, const Result &result) {
//                    executor::LocalTargetCallableRequest<Callable > request(instanceName, callable);
//                    try{
//                        invoke<serialization::pimpl::Void>(request);
//                        future.accessInternal().setValue(new Result(result));
//                    } catch(exception::ServerException &ex){
//                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
//                    } catch(...){
//                        logger<< "Exception in IExecuterService::asyncInvokeWithDefaultResult" << std::endl;
//                    }
//                }
//
//                template<typename Result, typename Request>
//                Result invoke(const Request &request, const Address &target) {
//                    return context->getInvocationService().invokeOnTarget<Result>(request, target);
//                }
//
//                template<typename Result, typename Request>
//                Result invoke(const Request &request) {
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
//
//        }
//    }
//}
//
//
//#endif //HAZELCAST_ExecutorDefaultResultImpl
