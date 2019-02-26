/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include <boost/foreach.hpp>

#include "ClientTestSupport.h"
#include "HazelcastServer.h"
#include <executor/tasks/CancellationAwareTask.h>
#include <executor/tasks/FailingCallable.h>
#include <executor/tasks/SelectNoMembers.h>
#include <executor/tasks/SerializedCounterCallable.h>
#include <executor/tasks/TaskWithUnserializableResponse.h>
#include <executor/tasks/GetMemberUuidTask.h>
#include <executor/tasks/AppendCallable.h>
#include <executor/tasks/SelectAllMembers.h>
#include <executor/tasks/MapPutPartitionAwareCallable.h>
#include <executor/tasks/NullCallable.h>

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/CountDownLatch.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExecutorServiceTest : public ClientTestSupport {
            protected:
                static const size_t numberOfMembers;

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory("hazelcast/test/resources/hazelcast-test-executor.xml");
                    for (size_t i = 0; i < numberOfMembers; ++i) {
                        instances.push_back(new HazelcastServer(*factory));
                    }
                    ClientConfig clientConfig;
                    client = new HazelcastClient(clientConfig);
                }

                static void TearDownTestCase() {
                    delete client;
                    BOOST_FOREACH(HazelcastServer *server, instances) {
                                    server->shutdown();
                                    delete server;
                                }

                    client = NULL;
                }

                class FailingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    FailingExecutionCallback(const boost::shared_ptr<util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const boost::shared_ptr<std::string> &response) {
                    }

                    virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                        latch->countDown();
                        exception = e;
                    }

                    boost::shared_ptr<exception::IException> getException() {
                        return exception.get();
                    }

                private:
                    const boost::shared_ptr<util::CountDownLatch> latch;
                    util::Atomic<boost::shared_ptr<exception::IException> > exception;
                };

                class SuccessfullExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    SuccessfullExecutionCallback(const boost::shared_ptr<util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const boost::shared_ptr<std::string> &response) {
                        latch->countDown();
                    }

                    virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                    }

                private:
                    const boost::shared_ptr<util::CountDownLatch> latch;
                };

                class ResultSettingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    ResultSettingExecutionCallback(const boost::shared_ptr<util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const boost::shared_ptr<std::string> &response) {
                        result.set(response);
                        latch->countDown();
                    }

                    virtual void onFailure(const boost::shared_ptr<exception::IException> &e) {
                    }

                    boost::shared_ptr<std::string> getResult() {
                        return result.get();
                    }

                private:
                    const boost::shared_ptr<util::CountDownLatch> latch;
                    util::Atomic<boost::shared_ptr<std::string> > result;
                };

                class MultiExecutionCompletionCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionCompletionCallback(const string &msg, const boost::shared_ptr<util::CountDownLatch> &responseLatch,
                                                     const boost::shared_ptr<util::CountDownLatch> &completeLatch) : msg(msg),
                                                                                      responseLatch(responseLatch),
                                                                                      completeLatch(completeLatch) {}

                    virtual void onResponse(const Member &member, const boost::shared_ptr<std::string> &response) {
                        if (response.get() && *response == msg + executor::tasks::AppendCallable::APPENDAGE) {
                            responseLatch->countDown();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, const boost::shared_ptr<exception::IException> &exception) {
                    }

                    virtual void onComplete(const std::map<Member, boost::shared_ptr<std::string> > &values,
                                            const std::map<Member, boost::shared_ptr<exception::IException> > &exceptions) {
                        typedef std::map<Member, boost::shared_ptr<std::string> > VALUE_MAP;
                        std::string expectedValue(msg + executor::tasks::AppendCallable::APPENDAGE);
                        BOOST_FOREACH(const VALUE_MAP::value_type &entry , values) {
                                        if (entry.second.get() && *entry.second == expectedValue) {
                                            completeLatch->countDown();
                                        }
                                    }
                    }
                private:
                    std::string msg;
                    const boost::shared_ptr<util::CountDownLatch> responseLatch;
                    const boost::shared_ptr<util::CountDownLatch> completeLatch;
                };

                class MultiExecutionNullCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionNullCallback(const boost::shared_ptr<util::CountDownLatch> &responseLatch, const boost::shared_ptr<util::CountDownLatch> &completeLatch) : responseLatch(responseLatch), completeLatch(completeLatch) {}

                    virtual void onResponse(const Member &member, const boost::shared_ptr<std::string> &response) {
                        if (response.get() == NULL) {
                            responseLatch->countDown();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, const boost::shared_ptr<exception::IException> &exception) {
                    }

                    virtual void onComplete(const std::map<Member, boost::shared_ptr<std::string> > &values,
                                            const std::map<Member, boost::shared_ptr<exception::IException> > &exceptions) {
                        typedef std::map<Member, boost::shared_ptr<std::string> > VALUE_MAP;
                        BOOST_FOREACH(const VALUE_MAP::value_type &entry , values) {
                                        if (entry.second.get() == NULL) {
                                            completeLatch->countDown();
                                        }
                                    }
                    }
                private:
                    const boost::shared_ptr<util::CountDownLatch> responseLatch;
                    const boost::shared_ptr<util::CountDownLatch> completeLatch;
                };

                static std::vector<HazelcastServer *> instances;
                static HazelcastClient *client;
                static HazelcastServerFactory *factory;
            };

            std::vector<HazelcastServer *>ClientExecutorServiceTest::instances;
            HazelcastClient *ClientExecutorServiceTest::client = NULL;
            HazelcastServerFactory *ClientExecutorServiceTest::factory = NULL;
            const size_t ClientExecutorServiceTest::numberOfMembers = 4;

            TEST_F(ClientExecutorServiceTest, testIsTerminated) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isTerminated());
            }

            TEST_F(ClientExecutorServiceTest, testIsShutdown) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdown) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdownMultipleTimes) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();
                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testCancellationAwareTask_whenTimeOut) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                boost::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                ASSERT_THROW(future->get(1, util::concurrent::TimeUnit::SECONDS()), exception::TimeoutException);
            }

            TEST_F(ClientExecutorServiceTest, testFutureAfterCancellationAwareTaskTimeOut) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                boost::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                try {
                    future->get(1, util::concurrent::TimeUnit::SECONDS());
                } catch (TimeoutException ignored) {
                }

                ASSERT_FALSE(future->isDone());
                ASSERT_FALSE(future->isCancelled());
            }

            TEST_F(ClientExecutorServiceTest, testGetFutureAfterCancel) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                boost::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                try {
                    future->get(1, util::concurrent::TimeUnit::SECONDS());
                } catch (TimeoutException ignored) {
                }

                ASSERT_TRUE(future->cancel(true));

                ASSERT_THROW(future->get(), exception::CancellationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::FailingCallable task;

                boost::shared_ptr<ICompletableFuture<std::string> > future = service->submit<executor::tasks::FailingCallable, std::string>(
                        task);

                ASSERT_THROW(future->get(), exception::ExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException_withExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                boost::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));

                executor::tasks::FailingCallable task;
                boost::shared_ptr<ExecutionCallback<std::string> > callback(new FailingExecutionCallback(latch));

                service->submit<executor::tasks::FailingCallable, std::string>(task, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableReasonExceptionCause) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                const boost::shared_ptr<ICompletableFuture<std::string> > &failingFuture = service->submit<executor::tasks::FailingCallable, std::string>(
                        executor::tasks::FailingCallable());

                try {
                    failingFuture->get();
                } catch (exception::ExecutionException &e) {
                    ASSERT_THROW(e.getCause()->raise(), exception::IllegalStateException);
                }
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withNoMemberSelected) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string mapName = randomMapName();

                executor::tasks::SelectNoMembers selector;

                ASSERT_THROW(service->execute<executor::tasks::MapPutPartitionAwareCallable>(executor::tasks::MapPutPartitionAwareCallable(mapName, randomString()),
                                                                               selector),
                             exception::RejectedExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce) {
                std::string name = getTestName();

                boost::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                boost::shared_ptr<ICompletableFuture<int> > future = service->submitToKeyOwner<executor::tasks::SerializedCounterCallable, int, std::string>(
                        counterCallable, name);
                boost::shared_ptr<int> value = future->get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce_submitToAddress) {
                std::string name = getTestName();

                boost::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_FALSE(members.empty());
                boost::shared_ptr<ICompletableFuture<int> > future = service->submitToMember<executor::tasks::SerializedCounterCallable, int>(
                        counterCallable, members[0]);
                boost::shared_ptr<int> value = future->get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClient) {
                std::string name = getTestName();

                boost::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                boost::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::TaskWithUnserializableResponse, bool>(
                        taskWithUnserializableResponse);

                try {
                    future->get();
                } catch (exception::ExecutionException &e) {
                    ASSERT_THROW(e.getCause()->raise(), exception::HazelcastSerializationException);
                }
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClientCallback) {
                std::string name = getTestName();

                boost::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                boost::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));

                boost::shared_ptr<FailingExecutionCallback> callback(new FailingExecutionCallback(latch));

                service->submit<executor::tasks::TaskWithUnserializableResponse, std::string>(
                        taskWithUnserializableResponse, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);

                boost::shared_ptr<exception::IException> exception = callback->getException();
                ASSERT_NOTNULL(exception.get(), exception::IException);
                ASSERT_THROW(exception->raise(), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMember) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                boost::shared_ptr<ICompletableFuture<std::string> > future = service->submitToMember<executor::tasks::GetMemberUuidTask, std::string>(
                        task, members[0]);

                boost::shared_ptr<std::string> uuid = future->get();
                ASSERT_NOTNULL(uuid.get(), std::string);
                ASSERT_EQ(members[0].getUuid(), *uuid);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::map<Member, boost::shared_ptr<ICompletableFuture<std::string> > > futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(
                        task, members);

                BOOST_FOREACH(const Member &member, members) {
                                ASSERT_EQ(1U, futuresMap.count(member));
                                boost::shared_ptr<std::string> uuid = futuresMap[member]->get();
                                ASSERT_NOTNULL(uuid.get(), std::string);
                                ASSERT_EQ(member.getUuid(), *uuid);
                            }
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withMemberSelector) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selectAll;

                boost::shared_ptr<ICompletableFuture<std::string> > f = service->submit<executor::tasks::AppendCallable, std::string>(callable, selectAll);

                boost::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers_withMemberSelector) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;
                executor::tasks::SelectAllMembers selectAll;

                typedef std::map<Member, boost::shared_ptr<ICompletableFuture<std::string> > > FUTURESMAP;
                FUTURESMAP futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(
                        task, selectAll);
                
                BOOST_FOREACH(const FUTURESMAP::value_type &pair, futuresMap) {
                                const Member &member = pair.first;
                                const boost::shared_ptr<ICompletableFuture<std::string> > &future = pair.second;

                                boost::shared_ptr<std::string> uuid = future->get();

                                ASSERT_NOTNULL(uuid.get(), std::string);
                                ASSERT_EQ(member.getUuid(), *uuid);
                            }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                typedef std::map<Member, boost::shared_ptr<ICompletableFuture<std::string> > > FUTURESMAP;
                FUTURESMAP futuresMap = service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable);

                BOOST_FOREACH(const FUTURESMAP::value_type &pair, futuresMap) {
                                const boost::shared_ptr<ICompletableFuture<std::string> > &future = pair.second;

                                boost::shared_ptr<std::string> result = future->get();

                                ASSERT_NOTNULL(result.get(), std::string);
                                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
                            }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withExecutionCallback) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, randomString());

                boost::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                boost::shared_ptr<SuccessfullExecutionCallback> callback(new SuccessfullExecutionCallback(latch));

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                service->submitToMember<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, members[0], callback);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);
                
                ASSERT_OPEN_EVENTUALLY(*latch);
                ASSERT_EQ(1, map.size());
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withMultiExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                boost::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                boost::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                boost::shared_ptr<MultiExecutionCallback<std::string> > callback(new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, members, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallable_withExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;
                boost::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(1));
                boost::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(responseLatch));

                service->submit<executor::tasks::AppendCallable, std::string>(callable, selector, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                boost::shared_ptr<std::string> message = callback->getResult();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMembers_withExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                boost::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                boost::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;

                boost::shared_ptr<MultiExecutionCallback<std::string> > callback(new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, selector, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers_withMultiExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                boost::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                boost::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                boost::shared_ptr<MultiExecutionCallback<std::string> > callback(new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*completeLatch);
                ASSERT_OPEN_EVENTUALLY(*responseLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableWithNullResultToAllMembers_withMultiExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                boost::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                boost::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                executor::tasks::NullCallable callable;

                boost::shared_ptr<MultiExecutionCallback<std::string> > callback(new MultiExecutionNullCallback(responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::NullCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                boost::shared_ptr<ICompletableFuture<std::string> > result = service->submit<executor::tasks::AppendCallable, std::string>(callable);

                boost::shared_ptr<std::string> message = result->get();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                boost::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                boost::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch));

                service->submit<executor::tasks::AppendCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
                boost::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                boost::shared_ptr<ICompletableFuture<std::string> > f = service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key");

                boost::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner_withExecutionCallback) {
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                boost::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                boost::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch));

                service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key", callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
                boost::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                boost::shared_ptr<ICompletableFuture<std::string> > f = service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable);

                boost::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(member.getUuid(), *result);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware_WithExecutionCallback) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                boost::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                boost::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch));

                service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
                boost::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(member.getUuid(), *value);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, testExecute) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"));

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withMemberSelector) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);
                executor::tasks::SelectAllMembers selector;

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"), selector);
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnKeyOwner) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string targetUuid = member.getUuid();
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                service->executeOnKeyOwner<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, key);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(targetUuid));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMember) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                Member &member = members[0];
                std::string targetUuid = member.getUuid();

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMember<executor::tasks::MapPutPartitionAwareCallable>(callable, member);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(targetUuid));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> allMembers = client->getCluster().getMembers();
                std::vector<Member> members(allMembers.begin(), allMembers.begin() + 2);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, members);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(members[0].getUuid()) && map.containsKey(members[1].getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withEmptyCollection) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable,
                                                                                         std::vector<Member>());

                assertSizeEventually(0, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withSelector) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                executor::tasks::SelectAllMembers selector;

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, selector);

                assertSizeEventually((int) numberOfMembers, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnAllMembers) {
                std::string testName = getTestName();
                boost::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnAllMembers<executor::tasks::MapPutPartitionAwareCallable>(callable);

                assertSizeEventually((int) numberOfMembers, map);
            }
        }
    }
}

