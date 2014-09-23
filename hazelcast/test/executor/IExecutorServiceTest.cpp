////
//// Created by sancar koyunlu on 9/6/13.
//// Copyright (c) 2013 hazelcast. All rights reserved.
//
//
//#include "executor/IExecutorServiceTest.h"
//#include "HazelcastServerFactory.h"
//#include "hazelcast/client/HazelcastClient.h"
//#include "executor/RunnableTask.h"
//#include "executor/CallableTask.h"
//#include "executor/FailingTask.h"
//
//namespace hazelcast {
//    namespace client {
//
//        class HazelcastClient;
//
//        namespace test {
//            using namespace iTest;
//
//            IExecutorServiceTest::IExecutorServiceTest(HazelcastServerFactory& hazelcastInstanceFactory)
//            :hazelcastInstanceFactory(hazelcastInstanceFactory)
//            , instance(hazelcastInstanceFactory)
//            , second(hazelcastInstanceFactory)
//            , third(hazelcastInstanceFactory)
//            , client(new HazelcastClient(clientConfig.addAddress(Address("localhost", 5701))))
//            , service(new IExecutorService(client->getExecutorService("IExecuterServiceTest"))) {
//
//            }
//
//
//            IExecutorServiceTest::~IExecutorServiceTest() {
//
//            }
//
//            void IExecutorServiceTest::addTests() {
//                addTest(&IExecutorServiceTest::testSubmitWithResult, "testSubmitWithResult");
//                addTest(&IExecutorServiceTest::submitCallable1, "submitCallable1");
//                addTest(&IExecutorServiceTest::submitCallable2, "submitCallable2");
//                addTest(&IExecutorServiceTest::submitCallable3, "submitCallable3");
//                addTest(&IExecutorServiceTest::submitCallable4, "submitCallable4");
//                addTest(&IExecutorServiceTest::submitFailingCallable, "submitFailingCallable");
//            }
//
//            void IExecutorServiceTest::beforeClass() {
//
//            }
//
//            void IExecutorServiceTest::afterClass() {
//                client.reset();
//                instance.shutdown();
//                second.shutdown();
//                third.shutdown();
//            }
//
//            void IExecutorServiceTest::beforeTest() {
//
//            }
//
//            void IExecutorServiceTest::afterTest() {
//            }
//
//            void IExecutorServiceTest::testSubmitWithResult() {
//                int res = 5;
//                std::string name = "task";
//                RunnableTask runnableTask(name);
//                Future<int> future = service->submit(runnableTask, res);
//                int integer = future.get();
//                assertEqual(res, integer);
//            }
//
//
//            void IExecutorServiceTest::submitCallable1() {
//                std::string name = "naber";
//                CallableTask callableTask(name);
//                Future<std::string> future = service->submitToKeyOwner<std::string>(callableTask, std::string("key"));
//                std::string& result = future.get();
//                assertEqual("naber:result", result);
//            }
//
//            class SampleExecutionCallback {
//            public:
//                SampleExecutionCallback(util::CountDownLatch& latch)
//                :latch(latch) {
//
//                }
//
//                void onResponse(std::string response) {
//                    if (!response.compare("naber:result")) {
//                        latch.countDown();
//                    }
//                }
//
//                void onFailure(std::exception t) {
//                }
//
//            private:
//                util::CountDownLatch& latch;
//            }
//
//            void IExecutorServiceTest::submitCallable2() {
//
//                util::CountDownLatch latch(1);
//                SampleExecutionCallback executionCallback(latch);
//                std::string name("naber");
//                std::string key("key");
//                CallableTask callableTask(name);
//                service->submitToKeyOwner<std::string>(callableTask, key, executionCallback);
//                assertTrue(latch.await((.*)));
//            }
//
//
//            void IExecutorServiceTest::submitCallable3() {
//                std::string name("asd");
//                CallableTask callableTask(name);
//                std::map<Member, Future<std::string> > map = service->submitToAllMembers<std::string>(callableTask);
//                std::map<Member, Future<std::string> >::iterator it;
//                for (it = map.begin(); it != map.end(); it++) {
//                    std::string & s = it->second.get();
//                    assertEqual("asd:result", s);
//                }
//            }
//
//            class SampleMultiExecutionCallback {
//            public:
//                SampleMultiExecutionCallback(util::CountDownLatch& latch)
//                :latch(latch) {
//
//                }
//
//                void onResponse(Member member, std::string response) {
//                    if (!response.compare("asd:result")) {
//                        latch.countDown();
//                    }
//                }
//
//                void onComplete(std::map<Member, std::string >& values) {
//                    std::map<Member, std::string >::iterator it;
//                    for (it = values.begin(); it != values.end(); it++) {
//                        std::string & s = it->second;
//                        if (!s.compare("asd:result")) {
//                            latch.countDown();
//                        }
//                    }
//                }
//
//            private:
//                util::CountDownLatch& latch;
//            }
//
//            void IExecutorServiceTest::submitCallable4() {
//                util::CountDownLatch latch(4);
//                std::string name("asd");
//                CallableTask callableTask(name);
//                SampleMultiExecutionCallback multiExecutionCallback(latch);
//                service->submitToAllMembers<std::string>(callableTask, multiExecutionCallback);
//                assertTrue(latch.await((.*)));
//            }
//
//
//            void IExecutorServiceTest::submitFailingCallable() {
//                std::cout << "Expected exception = > ";
//                FailingTask failingTask;
//                Future<std::string> f = service->submit<std::string>(failingTask);
//                try {
//                    std::string & get = f.get();
//                } catch (std::exception& e){
//                    std::cout << e.what() << std::endl;
//                }
//            }
//
//        }
//    }
//}

