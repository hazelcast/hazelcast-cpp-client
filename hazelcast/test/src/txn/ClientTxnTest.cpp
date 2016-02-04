/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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

#include "ClientTxnTest.h"
#include "HazelcastServerFactory.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class MyLoadBalancer : public impl::AbstractLoadBalancer {
            public:
                const Member next() {
                    std::vector<Member> members = getMembers();
                    long len = members.size();
                    if (len == 0) {
                        throw exception::IException("const Member& RoundRobinLB::next()", "No member in member list!!");
                    }
                    for (int i = 0; i < len; i++) {
                        if (members[i].getAddress().getPort() == 5701) {
                            return members[i];
                        }
                    }
                    return members[0];
                }

            };

            class MyLifecycleListener : public MembershipListener {
            public:
                MyLifecycleListener(util::CountDownLatch& countDownLatch)
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
                //always start the txn on first member
                loadBalancer.reset(new MyLoadBalancer());
                clientConfig->setLoadBalancer(loadBalancer.get());
                client.reset(new HazelcastClient(*clientConfig));
            }

            ClientTxnTest::~ClientTxnTest() {
                g_srvFactory->shutdownAll();
                client->shutdown();
                client.reset();                
            }

            TEST_F(ClientTxnTest, testTxnRollback) {
                util::CountDownLatch memberRemovedLatch(1);
                std::string queueName = "testTxnRollback";
                MyLifecycleListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);

                TransactionContext context = client->newTransactionContext();
                context.beginTransaction();
                TransactionalQueue<std::string> queue = context.getQueue<std::string>(queueName);
                queue.offer("item");
                server->shutdown();

                ASSERT_THROW(context.commitTransaction(), exception::IException);

                context.rollbackTransaction();

                ASSERT_TRUE(memberRemovedLatch.await(10));

                IQueue<std::string> q = client->getQueue<std::string>(queueName);
                try {
                    ASSERT_EQ(0, q.size());
                    ASSERT_EQ(q.poll().get(), (std::string *)NULL);
                } catch (exception::IException& e) {
                    std::cout << e.what() << std::endl;
                }
            }

            TEST_F(ClientTxnTest, testTxnRollbackOnServerCrash) {
                std::string queueName = "testTxnRollbackOnServerCrash";
                TransactionContext context = client->newTransactionContext();
                util::CountDownLatch memberRemovedLatch(1);
                context.beginTransaction();
                TransactionalQueue<std::string> queue = context.getQueue<std::string>(queueName);
                queue.offer("str");
                MyLifecycleListener myLifecycleListener(memberRemovedLatch);
                client->getCluster().addMembershipListener(&myLifecycleListener);
                server->shutdown();

                ASSERT_THROW(context.commitTransaction(), exception::IException);

                context.rollbackTransaction();

                ASSERT_TRUE(memberRemovedLatch.await(10));

                IQueue<std::string> q = client->getQueue<std::string>(queueName);
                ASSERT_EQ(q.poll().get(), (std::string *)NULL);
                ASSERT_EQ(0, q.size());
            }
        }
    }
}
