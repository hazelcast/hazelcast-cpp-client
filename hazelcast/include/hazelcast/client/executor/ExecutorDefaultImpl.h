////
//// Created by sancar koyunlu on 9/7/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//
//#ifndef HAZELCAST_ExecutorDefaultImpl
//#define HAZELCAST_ExecutorDefaultImpl
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
//
//            class ExecutorDefaultImpl {
//            public:
//                ExecutorDefaultImpl(const std::string &instanceName, spi::ClientContext *context)
//                :instanceName(instanceName), context(context) {
//
//                }
//
//                //----//
//                template<typename Result, typename Callable >
//                Future<Result> submit(Callable &task) {
//                    Future<Result> future;
//                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultImpl::asyncInvoke<Result, Callable>, this, boost::ref(task), future));
//                    return future;
//                }
//
//                template<typename Result, typename Runnable >
//                Future<Result> submit(executor::RunnableAdapter<Runnable> task) {
//                    Future<Result> future;
//                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultImpl::asyncInvoke<Result, executor::RunnableAdapter<Runnable> >, this, task, future));
//                    return future;
//                }
//
//                //----//
//                template<typename Result, typename Callable >
//                Future<Result> submit(Callable &task, const serialization::pimpl::Data &partitionKey) {
//                    spi::PartitionService &partitionService = context->getPartitionService();
//                    int partitionId = partitionService.getPartitionId(partitionKey);
//                    Address *pointer = partitionService.getPartitionOwner(partitionId);
//                    if (pointer != NULL)
//                        return submit<Result>(task, *pointer);
//                    else
//                        return submit<Result>(task);
//
//                }
//
//                //----//
//                template<typename Result, typename Callable >
//                Future<Result> submit(Callable &task, const Address &address) {
//                    Future<Result> future;
//                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultImpl::asyncInvokeToAddress<Result, Callable>, this, boost::ref(task), address, future));
//                    return future;
//                }
//
//                template<typename Result, typename Runnable >
//                Future<Result> submit(executor::RunnableAdapter<Runnable> task, const Address &address) {
//                    Future<Result> future;
//                    boost::thread asyncInvokeThread(boost::bind(&ExecutorDefaultImpl::asyncInvokeToAddress<Result, executor::RunnableAdapter<Runnable> >, this, task, address, future));
//                    return future;
//                }
//                //----//
//
//            private:
//
//                template<typename Result, typename Callable>
//                void asyncInvokeToAddress(Callable &callable, Address address, Future<Result> future) {
//
//                    executor::TargetCallableRequest<Callable > request(instanceName, callable, address);
//                    try {
//                        future.accessInternal().setValue(new Result(*(invoke<Result>(request, address))));
//                    } catch(exception::ServerException &ex) {
//                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
//                    } catch(...) {
//                        std::cerr << "Exception in IExecuterService::asyncInvoke" << std::endl;
//                    }
//                }
//
//                template<typename Result, typename Callable>
//                void asyncInvoke(Callable &callable, Future<Result> future) {
//                    executor::LocalTargetCallableRequest< Callable > request(instanceName, callable);
//                    try {
//                        future.accessInternal().setValue(new Result(*(invoke<Result>(request))));
//                    } catch(exception::ServerException &ex) {
//                        future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
//                    } catch(...) {
//                        std::cerr << "Exception in IExecuterService::asyncInvoke" << std::endl;
//                    }
//                }
//
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
//
//            };
//
//        }
//    }
//}
//
//#endif //HAZELCAST_ExecutorDefaultImpl
