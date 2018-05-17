/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 9/18/13.

/**
 * This has to be the first include, so that Python.h is the first include. Otherwise, compilation warning such as
 * "_POSIX_C_SOURCE" redefined occurs.
 */
#include "HazelcastServerFactory.h"

#include "ClientTestSupport.h"
#include "HazelcastServer.h"

#include "hazelcast/client/HazelcastClient.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientTxnTest : public ClientTestSupport {
            public:
                ClientTxnTest();

                ~ClientTxnTest();

            protected:
                HazelcastServerFactory & hazelcastInstanceFactory;
                std::auto_ptr<HazelcastServer> server;
                std::auto_ptr<HazelcastServer> second;
                std::auto_ptr<HazelcastClient> client;
                std::auto_ptr<LoadBalancer> loadBalancer;
            };

            class MyLoadBalancer : public impl::AbstractLoadBalancer {
            public:
                const Member next() {
                    std::vector<Member> members = getMembers();
                    size_t len = members.size();
                    if (len == 0) {
                        throw exception::IOException("const Member& RoundRobinLB::next()", "No member in member list!!");
                    }
                    for (size_t i = 0; i < len; i++) {
                        if (members[i].getAddress().getPort() == 5701) {
                            return members[i];
                        }
                    }
                    return members[0];
                }

            };

            class MyMembershipListener : public MembershipListener {
            public:
                MyMembershipListener(util::CountDownLatch& countDownLatch)
                : countDownLatch(countDownLatch) {

                }

                void memberAdded(const MembershipEvent& membershipEvent) {

                }

                void memberRemoved(const MembershipEvent& membershipEvent) {
                    countDownLatch.countDown();
                }

                void memberAttributeChanged(const MemberAttributeEvent& memberAttributeEvent) {

                }

            private:
                util::CountDownLatch& countDownLatch;
            };

            ClientTxnTest::ClientTxnTest()
            : hazelcastInstanceFactory(*g_srvFactory) {
                server.reset(new HazelcastServer(hazelcastInstanceFactory));
                second.reset(new HazelcastServer(hazelcastInstanceFactory));
                std::auto_ptr<ClientConfig> clientConfig = getConfig();
                clientConfig->setRedoOperation(true);
                // Keep the test time shorter by setting a shorter invocation timeout
                clientConfig->getProperties()[ClientProperties::INVOCATION_TIMEOUT_SECONDS] = "5";
                //always start the txn on first member
                loadBalancer.reset(new MyLoadBalancer());
                clientConfig->setLoadBalancer(loadBalancer.get());
                client.reset(new HazelcastClient(*clientConfig));
            }

            ClientTxnTest::~ClientTxnTest() {
                client->shutdown();
                client.reset();
            }

            TEST_F(ClientTxnTest, testTxnRollback) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                util::CountDownLatch txnRollbackLatch(1);
                util::CountDownLatch memberRemovedLatch(1);
                MyMembershipListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);

                try {
                    context.beginTransaction();
                    ASSERT_FALSE(context.getTxnId().empty());
                    TransactionalQueue<std::string> queue = context.getQueue<std::string>(queueName);
                    queue.offer(randomString());

                    server->shutdown();

                    context.commitTransaction();
                    FAIL();
                } catch (exception::TransactionException &e) {
                    context.rollbackTransaction();
                    txnRollbackLatch.countDown();
                }

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                IQueue <string> q = client->getQueue<std::string>(queueName);
                ASSERT_NULL("Poll result should be null since it is rolled back", q.poll().get(), std::string);
                ASSERT_EQ(0, q.size());
            }

            TEST_F(ClientTxnTest, testTxnRollbackOnServerCrash) {
                std::string queueName = randomString();
                TransactionContext context = client->newTransactionContext();
                util::CountDownLatch txnRollbackLatch(1);
                util::CountDownLatch memberRemovedLatch(1);

                context.beginTransaction();

                TransactionalQueue<std::string> queue = context.getQueue<std::string>(queueName);
                queue.offer("str");

                MyMembershipListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);

                server->shutdown();

                ASSERT_THROW(context.commitTransaction(), exception::TransactionException);

                context.rollbackTransaction();
                txnRollbackLatch.countDown();

                ASSERT_OPEN_EVENTUALLY(txnRollbackLatch);
                ASSERT_OPEN_EVENTUALLY(memberRemovedLatch);

                IQueue<std::string> q = client->getQueue<std::string>(queueName);
                ASSERT_NULL("queue poll should return null", q.poll().get(), std::string);
                ASSERT_EQ(0, q.size());
            }
        }
    }
}
