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


#include "HazelcastServerFactory.h"
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
#include "hazelcast/util/Sync.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientExecutorServiceTest : public ClientTestSupport {
            protected:
                static const size_t numberOfMembers;

                virtual void TearDown() {
                }

                static void SetUpTestCase() {
                    factory = new HazelcastServerFactory(g_srvFactory->getServerAddress(),
                                                         "hazelcast/test/resources/hazelcast-test-executor.xml");
                    for (size_t i = 0; i < numberOfMembers; ++i) {
                        instances.push_back(new HazelcastServer(*factory));
                    }
                    client = new HazelcastClient;
                }

                static void TearDownTestCase() {
                    delete client;
                    for (HazelcastServer *server : instances) {
                        server->shutdown();
                        delete server;
                    }

                    client = NULL;
                }

                class FailingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    FailingExecutionCallback(const std::shared_ptr<util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                    }

                    virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                        latch->countDown();
                        exception = e;
                    }

                    std::shared_ptr<exception::IException> getException() {
                        return exception.get();
                    }

                private:
                    const std::shared_ptr<util::CountDownLatch> latch;
                    util::Sync<std::shared_ptr<exception::IException> > exception;
                };

                class SuccessfullExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    SuccessfullExecutionCallback(const std::shared_ptr<util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                        latch->countDown();
                    }

                    virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                    }

                private:
                    const std::shared_ptr<util::CountDownLatch> latch;
                };

                class ResultSettingExecutionCallback : public ExecutionCallback<std::string> {
                public:
                    ResultSettingExecutionCallback(const std::shared_ptr<util::CountDownLatch> &latch) : latch(latch) {}

                    virtual void onResponse(const std::shared_ptr<std::string> &response) {
                        result.set(response);
                        latch->countDown();
                    }

                    virtual void onFailure(const std::shared_ptr<exception::IException> &e) {
                    }

                    std::shared_ptr<std::string> getResult() {
                        return result.get();
                    }

                private:
                    const std::shared_ptr<util::CountDownLatch> latch;
                    util::Sync<std::shared_ptr<std::string>> result;
                };

                class MultiExecutionCompletionCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionCompletionCallback(const string &msg,
                                                     const std::shared_ptr<util::CountDownLatch> &responseLatch,
                                                     const std::shared_ptr<util::CountDownLatch> &completeLatch) : msg(
                            msg),
                                                                                                                   responseLatch(
                                                                                                                           responseLatch),
                                                                                                                   completeLatch(
                                                                                                                           completeLatch) {}

                    virtual void onResponse(const Member &member, const std::shared_ptr<std::string> &response) {
                        if (response.get() && *response == msg + executor::tasks::AppendCallable::APPENDAGE) {
                            responseLatch->countDown();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, const std::shared_ptr<exception::IException> &exception) {
                    }

                    virtual void onComplete(const std::map<Member, std::shared_ptr<std::string> > &values,
                                            const std::map<Member, std::shared_ptr<exception::IException> > &exceptions) {
                        typedef std::map<Member, std::shared_ptr<std::string> > VALUE_MAP;
                        std::string expectedValue(msg + executor::tasks::AppendCallable::APPENDAGE);
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second.get() && *entry.second == expectedValue) {
                                completeLatch->countDown();
                            }
                        }
                    }

                private:
                    std::string msg;
                    const std::shared_ptr<util::CountDownLatch> responseLatch;
                    const std::shared_ptr<util::CountDownLatch> completeLatch;
                };

                class MultiExecutionNullCallback : public MultiExecutionCallback<std::string> {
                public:
                    MultiExecutionNullCallback(const std::shared_ptr<util::CountDownLatch> &responseLatch,
                                               const std::shared_ptr<util::CountDownLatch> &completeLatch)
                            : responseLatch(responseLatch), completeLatch(completeLatch) {}

                    virtual void onResponse(const Member &member, const std::shared_ptr<std::string> &response) {
                        if (response.get() == NULL) {
                            responseLatch->countDown();
                        }
                    }

                    virtual void
                    onFailure(const Member &member, const std::shared_ptr<exception::IException> &exception) {
                    }

                    virtual void onComplete(const std::map<Member, std::shared_ptr<std::string> > &values,
                                            const std::map<Member, std::shared_ptr<exception::IException> > &exceptions) {
                        typedef std::map<Member, std::shared_ptr<std::string> > VALUE_MAP;
                        for (const VALUE_MAP::value_type &entry  : values) {
                            if (entry.second.get() == NULL) {
                                completeLatch->countDown();
                            }
                        }
                    }

                private:
                    const std::shared_ptr<util::CountDownLatch> responseLatch;
                    const std::shared_ptr<util::CountDownLatch> completeLatch;
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
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isTerminated());
            }

            TEST_F(ClientExecutorServiceTest, testIsShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                ASSERT_FALSE(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdown) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testShutdownMultipleTimes) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                service->shutdown();
                service->shutdown();

                ASSERT_TRUE_EVENTUALLY(service->isShutdown());
            }

            TEST_F(ClientExecutorServiceTest, testCancellationAwareTask_whenTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                ASSERT_THROW(future->get(1, util::concurrent::TimeUnit::SECONDS()), exception::TimeoutException);
            }

            TEST_F(ClientExecutorServiceTest, testFutureAfterCancellationAwareTaskTimeOut) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                try {
                    future->get(1, util::concurrent::TimeUnit::SECONDS());
                } catch (TimeoutException &ignored) {
                }

                ASSERT_FALSE(future->isDone());
                ASSERT_FALSE(future->isCancelled());
            }

            TEST_F(ClientExecutorServiceTest, testGetFutureAfterCancel) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::CancellationAwareTask task(INT64_MAX);

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::CancellationAwareTask, bool>(
                        task);

                try {
                    future->get(1, util::concurrent::TimeUnit::SECONDS());
                } catch (TimeoutException &ignored) {
                }

                ASSERT_TRUE(future->cancel(true));

                ASSERT_THROW(future->get(), exception::CancellationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::FailingCallable task;

                std::shared_ptr<ICompletableFuture<std::string> > future = service->submit<executor::tasks::FailingCallable, std::string>(
                        task);

                ASSERT_THROW(future->get(), exception::ExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableException_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));

                executor::tasks::FailingCallable task;
                std::shared_ptr<ExecutionCallback<std::string> > callback(new FailingExecutionCallback(latch));

                service->submit<executor::tasks::FailingCallable, std::string>(task, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitFailingCallableReasonExceptionCause) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                const std::shared_ptr<ICompletableFuture<std::string> > &failingFuture = service->submit<executor::tasks::FailingCallable, std::string>(
                        executor::tasks::FailingCallable());

                try {
                    failingFuture->get();
                } catch (exception::ExecutionException &e) {
                    ASSERT_THROW(e.getCause()->raise(), exception::IllegalStateException);
                }
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withNoMemberSelected) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string mapName = randomMapName();

                executor::tasks::SelectNoMembers selector;

                ASSERT_THROW(service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(mapName, randomString()),
                        selector),
                             exception::RejectedExecutionException);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                std::shared_ptr<ICompletableFuture<int> > future = service->submitToKeyOwner<executor::tasks::SerializedCounterCallable, int, std::string>(
                        counterCallable, name);
                std::shared_ptr<int> value = future->get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testCallableSerializedOnce_submitToAddress) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::SerializedCounterCallable counterCallable;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_FALSE(members.empty());
                std::shared_ptr<ICompletableFuture<int> > future = service->submitToMember<executor::tasks::SerializedCounterCallable, int>(
                        counterCallable, members[0]);
                std::shared_ptr<int> value = future->get();
                ASSERT_NOTNULL(value.get(), int);
                ASSERT_EQ(2, *value);
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClient) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                std::shared_ptr<ICompletableFuture<bool> > future = service->submit<executor::tasks::TaskWithUnserializableResponse, bool>(
                        taskWithUnserializableResponse);

                try {
                    future->get();
                } catch (exception::ExecutionException &e) {
                    ASSERT_THROW(e.getCause()->raise(), exception::HazelcastSerializationException);
                }
            }

            TEST_F(ClientExecutorServiceTest, testUnserializableResponse_exceptionPropagatesToClientCallback) {
                std::string name = getTestName();

                std::shared_ptr<IExecutorService> service = client->getExecutorService(name);

                executor::tasks::TaskWithUnserializableResponse taskWithUnserializableResponse;

                std::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));

                std::shared_ptr<FailingExecutionCallback> callback(new FailingExecutionCallback(latch));

                service->submit<executor::tasks::TaskWithUnserializableResponse, std::string>(
                        taskWithUnserializableResponse, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);

                std::shared_ptr<exception::IException> exception = callback->getException();
                ASSERT_NOTNULL(exception.get(), exception::IException);
                ASSERT_THROW(exception->raise(), exception::HazelcastSerializationException);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMember) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::shared_ptr<ICompletableFuture<std::string> > future = service->submitToMember<executor::tasks::GetMemberUuidTask, std::string>(
                        task, members[0]);

                std::shared_ptr<std::string> uuid = future->get();
                ASSERT_NOTNULL(uuid.get(), std::string);
                ASSERT_EQ(members[0].getUuid(), *uuid);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::map<Member, std::shared_ptr<ICompletableFuture<std::string> > > futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(
                        task, members);

                for (const Member &member : members) {
                    ASSERT_EQ(1U, futuresMap.count(member));
                    std::shared_ptr<std::string> uuid = futuresMap[member]->get();
                    ASSERT_NOTNULL(uuid.get(), std::string);
                    ASSERT_EQ(member.getUuid(), *uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selectAll;

                std::shared_ptr<ICompletableFuture<std::string> > f = service->submit<executor::tasks::AppendCallable, std::string>(
                        callable, selectAll);

                std::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallableToMembers_withMemberSelector) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                executor::tasks::GetMemberUuidTask task;
                executor::tasks::SelectAllMembers selectAll;

                typedef std::map<Member, std::shared_ptr<ICompletableFuture<std::string> > > FUTURESMAP;
                FUTURESMAP futuresMap = service->submitToMembers<executor::tasks::GetMemberUuidTask, std::string>(
                        task, selectAll);

                for (const FUTURESMAP::value_type &pair : futuresMap) {
                    const Member &member = pair.first;
                    const std::shared_ptr<ICompletableFuture<std::string> > &future = pair.second;

                    std::shared_ptr<std::string> uuid = future->get();

                    ASSERT_NOTNULL(uuid.get(), std::string);
                    ASSERT_EQ(member.getUuid(), *uuid);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                typedef std::map<Member, std::shared_ptr<ICompletableFuture<std::string> > > FUTURESMAP;
                FUTURESMAP futuresMap = service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(
                        callable);

                for (const FUTURESMAP::value_type &pair : futuresMap) {
                    const std::shared_ptr<ICompletableFuture<std::string> > &future = pair.second;

                    std::shared_ptr<std::string> result = future->get();

                    ASSERT_NOTNULL(result.get(), std::string);
                    ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
                }
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, randomString());

                std::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                std::shared_ptr<SuccessfullExecutionCallback> callback(new SuccessfullExecutionCallback(latch));

                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                service->submitToMember<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable,
                                                                                                    members[0],
                                                                                                    callback);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                ASSERT_OPEN_EVENTUALLY(*latch);
                ASSERT_EQ(1, map.size());
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMember_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                std::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                std::vector<Member> members = client->getCluster().getMembers();
                ASSERT_EQ(numberOfMembers, members.size());

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, members, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;
                std::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(
                        new ResultSettingExecutionCallback(responseLatch));

                service->submit<executor::tasks::AppendCallable, std::string>(callable, selector,
                                                                              static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                std::shared_ptr<std::string> message = callback->getResult();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToMembers_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                std::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);
                executor::tasks::SelectAllMembers selector;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToMembers<executor::tasks::AppendCallable, std::string>(callable, selector, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                std::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionCompletionCallback(msg, responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::AppendCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*completeLatch);
                ASSERT_OPEN_EVENTUALLY(*responseLatch);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableWithNullResultToAllMembers_withMultiExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::shared_ptr<util::CountDownLatch> responseLatch(new util::CountDownLatch(numberOfMembers));
                std::shared_ptr<util::CountDownLatch> completeLatch(new util::CountDownLatch(numberOfMembers));

                executor::tasks::NullCallable callable;

                std::shared_ptr<MultiExecutionCallback<std::string> > callback(
                        new MultiExecutionNullCallback(responseLatch, completeLatch));

                service->submitToAllMembers<executor::tasks::NullCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*responseLatch);
                ASSERT_OPEN_EVENTUALLY(*completeLatch);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<ICompletableFuture<std::string> > result = service->submit<executor::tasks::AppendCallable, std::string>(
                        callable);

                std::shared_ptr<std::string> message = result->get();
                ASSERT_NOTNULL(message.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *message);
            }

            TEST_F(ClientExecutorServiceTest, testSubmitCallable_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch));

                service->submit<executor::tasks::AppendCallable, std::string>(callable,
                                                                              static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                      callback));

                ASSERT_OPEN_EVENTUALLY(*latch);
                std::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<ICompletableFuture<std::string> > f = service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(
                        callable, "key");

                std::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *result);
            }

            TEST_F(ClientExecutorServiceTest, submitCallableToKeyOwner_withExecutionCallback) {
                std::shared_ptr<IExecutorService> service = client->getExecutorService(getTestName());

                std::string msg = randomString();
                executor::tasks::AppendCallable callable(msg);

                std::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                std::shared_ptr<ResultSettingExecutionCallback> callback(new ResultSettingExecutionCallback(latch));

                service->submitToKeyOwner<executor::tasks::AppendCallable, std::string, std::string>(callable, "key",
                                                                                                     static_pointer_cast<ExecutionCallback<std::string>>(
                                                                                                             callback));

                ASSERT_OPEN_EVENTUALLY(*latch);
                std::shared_ptr<std::string> value = callback->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(msg + executor::tasks::AppendCallable::APPENDAGE, *value);
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                std::shared_ptr<ICompletableFuture<std::string> > f = service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(
                        callable);

                std::shared_ptr<std::string> result = f->get();
                ASSERT_NOTNULL(result.get(), std::string);
                ASSERT_EQ(member.getUuid(), *result);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, submitCallablePartitionAware_WithExecutionCallback) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> members = client->getCluster().getMembers();
                spi::ClientContext clientContext(*client);
                Member &member = members[0];
                std::string key = generateKeyOwnedBy(clientContext, member);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, key);

                std::shared_ptr<util::CountDownLatch> latch(new util::CountDownLatch(1));
                std::shared_ptr<ExecutionCallback<std::string>> callback(new ResultSettingExecutionCallback(latch));

                service->submit<executor::tasks::MapPutPartitionAwareCallable, std::string>(callable, callback);

                ASSERT_OPEN_EVENTUALLY(*latch);
                std::shared_ptr<std::string> value = static_pointer_cast<ResultSettingExecutionCallback>(
                        callback)->getResult();
                ASSERT_NOTNULL(value.get(), std::string);
                ASSERT_EQ(member.getUuid(), *value);
                ASSERT_TRUE(map.containsKey(member.getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, testExecute) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"));

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecute_withMemberSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);
                executor::tasks::SelectAllMembers selector;

                service->execute<executor::tasks::MapPutPartitionAwareCallable>(
                        executor::tasks::MapPutPartitionAwareCallable(testName, "key"), selector);
                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                assertSizeEventually(1, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnKeyOwner) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

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
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

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
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                std::vector<Member> allMembers = client->getCluster().getMembers();
                std::vector<Member> members(allMembers.begin(), allMembers.begin() + 2);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, members);

                ASSERT_TRUE_EVENTUALLY(map.containsKey(members[0].getUuid()) && map.containsKey(members[1].getUuid()));
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withEmptyCollection) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable,
                                                                                         std::vector<Member>());

                assertSizeEventually(0, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnMembers_withSelector) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                executor::tasks::SelectAllMembers selector;

                service->executeOnMembers<executor::tasks::MapPutPartitionAwareCallable>(callable, selector);

                assertSizeEventually((int) numberOfMembers, map);
            }

            TEST_F(ClientExecutorServiceTest, testExecuteOnAllMembers) {
                std::string testName = getTestName();
                std::shared_ptr<IExecutorService> service = client->getExecutorService(testName);

                IMap<std::string, std::string> map = client->getMap<std::string, std::string>(testName);

                executor::tasks::MapPutPartitionAwareCallable callable(testName, "key");

                service->executeOnAllMembers<executor::tasks::MapPutPartitionAwareCallable>(callable);

                assertSizeEventually((int) numberOfMembers, map);
            }
        }
    }
}

