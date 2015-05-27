//
// Created by sancar koyunlu on 21/04/14.
//

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "HazelcastServerFactory.h"
#include "issues/IssueTest.h"
#include "HazelcastServer.h"

namespace hazelcast {
    namespace client {
        namespace test {

            IssueTest::IssueTest(HazelcastServerFactory &serverFactory)
            : ClientTestSupport<IssueTest>("IssueTest")
            , serverFactory(serverFactory) {

            }

            IssueTest::~IssueTest() {

            }

            void IssueTest::addTests() {
                addTest(&IssueTest::testOperationRedo_smartRoutingDisabled, "testOperationRedo_smartRoutingDisabled");
            }

            void IssueTest::beforeClass() {

            }

            void IssueTest::afterClass() {

            }

            void IssueTest::beforeTest() {

            }

            void IssueTest::afterTest() {

            }

            void threadTerminateNode(util::ThreadArgs &args) {
                HazelcastServer *node = (HazelcastServer *) args.arg0;
                node->shutdown();
            }

            void IssueTest::testOperationRedo_smartRoutingDisabled() {
                HazelcastServer hz1(serverFactory);
                HazelcastServer hz2(serverFactory);

                std::auto_ptr<ClientConfig> clientConfig(getConfig());
                clientConfig->setRedoOperation(true);
                clientConfig->setSmart(false);

                HazelcastClient client(*clientConfig);

                client::IMap<int, int> map = client.getMap<int, int>("m");
                util::Thread* thread = NULL;
                int expected = 1000;
                for (int i = 0; i < expected; i++) {
                    if(i == 5){
                        thread = new util::Thread(threadTerminateNode, &hz1);
                    }
                    map.put(i, i);
                }
                thread->join();
                delete thread;
                iTest::assertEqual(expected, map.size());
            }
        }
    }
}
