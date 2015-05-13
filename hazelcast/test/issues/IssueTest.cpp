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
            : iTest::iTestFixture<IssueTest>("IssueTest")
            , serverFactory(serverFactory), latch(2), listener(latch) {

            }

            IssueTest::~IssueTest() {

            }

            void IssueTest::addTests() {
                addTest(&IssueTest::testOperationRedo_smartRoutingDisabled, "testOperationRedo_smartRoutingDisabled");
                addTest(&IssueTest::testListenerSubscriptionOnSingleServerRestart,
                        "testListenerSubscriptionOnSingleServerRestart");
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

            void putMapMessage(util::ThreadArgs &args) {
                std::cout << __FILE__ << ":" << __LINE__ << " [putMapMessage] Started." << std::endl;

                IMap<int, int> *map = (IMap<int, int> *)args.arg0;
                util::CountDownLatch *latch = (util::CountDownLatch *)args.arg1;

                do {
                    // 7. Put a 2nd entry to the map
                    map->put(2, 20);
                    sleep(1);
                } while (latch->get() > 0);

                std::cout << __FILE__ << ":" << __LINE__ << " [putMapMessage] Finished." << std::endl;
            }

            void IssueTest::testOperationRedo_smartRoutingDisabled() {
                HazelcastServer hz1(serverFactory);
                HazelcastServer hz2(serverFactory);

                ClientConfig clientConfig;
                Address address = Address(serverFactory.getServerAddress(), 5701);
                clientConfig.addAddress(address);
                clientConfig.setRedoOperation(true);
                clientConfig.setSmart(false);

                HazelcastClient client(clientConfig);

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

            void IssueTest::testListenerSubscriptionOnSingleServerRestart() {
                HazelcastServer server(serverFactory);

                // 2. Start a client
                ClientConfig clientConfig;
                Address address = Address(serverFactory.getServerAddress(), 5701);
                clientConfig.addAddress(address);
                clientConfig.setConnectionAttemptLimit(10);

                HazelcastClient client(clientConfig);

                // 3. Get a map
                IMap <int, int> map = client.getMap<int, int>("IssueTest_map");

                // 4. Subscribe client to entry added event
                map.addEntryListener(listener, true);

                // Put a key, value to the map
                iTest::assertEqual((int *)NULL, map.put(1, 10).get());

                sleep(1);

                // 5. Verify that the listener got the entry added event
                iTest::assertEqual(1, latch.get());

                // 6. Restart the server
                iTest::assertTrue(server.shutdown());
                iTest::assertTrue(server.start());

                std::string putThreadName("Map Put Thread");
                util::Thread t(putThreadName, putMapMessage, &map, &latch);

                // 8. Verify that the 2nd entry is received by the listener
                iTest::assertTrue(latch.await(20, 0)); // timeout of 20 seconds

                t.interrupt();
                t.join();

                // 9. Shut down the server
                iTest::assertTrue(server.shutdown());
            }

            void IssueTest::Issue864MapListener::entryAdded(const EntryEvent<int, int> &event) {
                std::cout << __FILE__ << ":" << __LINE__ << " [Issue864MapListener::entryAdded] ENTRY. Key:" << event.getKey() << std::endl;

                int count = latch.get();
                if (2 == count) {
                    // The received event should be the addition of key value: 1, 10
                    iTest::assertEqual(1, event.getKey());
                    iTest::assertEqual(10, event.getValue());
                } else if (1 == count) {
                    // The received event should be the addition of key value: 2, 20
                    iTest::assertEqual(2, event.getKey());
                    iTest::assertEqual(20, event.getValue());
                }

                latch.countDown();

                std::cout << __FILE__ << ":" << __LINE__ << " [Issue864MapListener::entryAdded] EXIT. latch count:" << latch.get() << std::endl;
            }

            void IssueTest::Issue864MapListener::entryRemoved(const EntryEvent<int, int> &event) {

            }

            void IssueTest::Issue864MapListener::entryUpdated(const EntryEvent<int, int> &event) {
                std::cout << __FILE__ << ":" << __LINE__ << " [Issue864MapListener::entryUpdated] ENTRY. latch count:" << latch.get() << std::endl;
                iTest::assertEqual(2, event.getKey());
                iTest::assertEqual(20, event.getValue());
                latch.countDown();

                std::cout << __FILE__ << ":" << __LINE__ << " [Issue864MapListener::entryUpdated] EXIT. latch count:" << latch.get() << std::endl;
            }

            void IssueTest::Issue864MapListener::entryEvicted(const EntryEvent<int, int> &event) {

            }

            void IssueTest::Issue864MapListener::mapEvicted(const MapEvent &event) {

            }

            void IssueTest::Issue864MapListener::mapCleared(const MapEvent &event) {

            }

            IssueTest::Issue864MapListener::Issue864MapListener(util::CountDownLatch &l) : latch(l) {

            }
        }
    }
}
