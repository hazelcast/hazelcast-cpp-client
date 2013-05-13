//#ifndef MAPTEST
//#define MAPTEST
//
//
//#include "Socket.h"
//#include "TestPortableFactory.h"
//#include "SerializationService.h"
//#include "SimpleMapTest.h"
//#include "OutputSocketStream.h"
//#include "MapPutOperation.h"
//#include "testUtil.h"
//#include <fstream>
//
//
//void testPut() {
//    serialization::SerializationService service(1, getPortableFactoryMap());
//    Data key = service.toData(23);
//    Data value = service.toData(32);
//    MapPutOperation mapPutOperation(key, value);
//    Address address(SERVER_ADDRESS, SERVER_PORT);
//    hazelcast::client::protocol::Socket socket(address);
//    char protocol[] = {'C', 'B', '1'};
//    socket.send((void *) protocol, 3 * sizeof(char));
//
//    Data data = service.toData(mapPutOperation);
//
//    DataOutput output(new OutputSocketStream(socket));
//    data.writeData(output);
//}
//
//void testSpeed() {
//    SimpleMapTest s(SERVER_ADDRESS, SERVER_PORT);
//    s.run();
//};
//
//void testMapOperations() {
//    TestPortableFactory tpf1;
//    ClientConfig clientConfig(Address(SERVER_ADDRESS, SERVER_PORT));
//    clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");
//    clientConfig.addPortableFactory(999, &tpf1);
//
//    try {
//
//        HazelcastClient hazelcastClient(clientConfig);
//        IMap<int, TestMainPortable> imap = hazelcastClient.getMap<int, TestMainPortable >("sancar");
//        TestMainPortable mainPortable = getTestMainPortable();
//        for (int i = 0; i < 100; i++) {
//            TestMainPortable x = mainPortable;
//            x.i = i * 10;
//            x.p.ii.push_back(i * 100);
//            imap.put(i, x);
//        }
//
//        for (int i = 0; i < 100; i++) {
//            TestMainPortable x = imap.get(i);
//            assert(x.i == i * 10);
//            assert(x.p.ii.at(x.p.ii.size() - 1) == i * 100);
//        }
//
//        assert(imap.containsKey(2) == true);
//        assert(imap.containsKey(120) == false);
//
//        TestMainPortable temp = imap.get(5);
//        assert(imap.containsValue(temp) == true);
//        temp.i = 2;
//        assert(imap.containsValue(temp) == false);
//
//        set<int> keys = imap.keySet();
//        int i = 0;
//        for (set<int>::iterator it = keys.begin(); it != keys.end(); it++) {
//            assert(*it == i++);
//        }
//        vector<TestMainPortable> values = imap.values();
//        i = 0;
//        for (vector<TestMainPortable>::iterator it = values.begin(); it != values.end(); it++) {
//            assert(it->i == i * 10);
//            assert(it->p.ii.at(it->p.ii.size() - 1) == i * 100);
//            i++;
//        }
//
//        int myints[] = {0, 2, 4, 6};
//        std::set<int> keySet(myints, myints + sizeof (myints) / sizeof (int));
//        std::map<int, TestMainPortable> stdMap = imap.getAll(keySet);
//        for (int i = 0; i < 8; i += 2) {
//            TestMainPortable x = stdMap[i];
//            assert(x.i == i * 10);
//            assert(x.p.ii.at(x.p.ii.size() - 1) == i * 100);
//        }
//
//        std::pair<int, TestMainPortable> entry = imap.getEntry(3);
//        assert(entry.first == 3);
//        assert(entry.second.i = 30);
//        assert(entry.second.p.ii.at(entry.second.p.ii.size() - 1) == 300);
//
//        std::vector< std::pair<int, TestMainPortable> > entrySet = imap.entrySet();
//        std::vector< std::pair<int, TestMainPortable> >::iterator it;
//        i = 0;
//        for (it = entrySet.begin(); it != entrySet.end(); it++) {
//            assert(it->first == i);
//            assert(it->second.i == i * 10);
//            assert(it->second.p.ii.at(it->second.p.ii.size() - 1) == i * 100);
//            i++;
//        }
//
//        imap.remove(9);
//        assert(imap.containsKey(9) == false);
//
//        assert(imap.tryRemove(8, 1000) == true);
//        assert(imap.containsKey(8) == false);
//        //        imap.flush(); TODO
//        assert(imap.evict(8) == false);
//        assert(imap.evict(7) == true);
//        assert(imap.containsKey(7) == false);
//
//
//        assert(imap.tryPut(7, temp, 1000) == true);
//        assert(imap.containsKey(7) == true);
//
//        imap.put(8, temp, 0);
//        assert(imap.containsKey(8) == true);
//
//        imap.putTransient(9, temp, 0);
//        assert(imap.containsKey(9) == true);
//
//        TestMainPortable value6 = imap.get(6);
//        assert(imap.replace(6, value6, temp) == true);
//        assert(imap.get(6) == temp);
//        assert(imap.replace(2, value6, temp) == false);
//        assert(imap.get(2) != temp);
//
//    } catch (std::exception& e) {
//        std::cout << e.what() << std::endl;
//    }
//};
//
//void testMapLocksInSequential() {
//    ClientConfig clientConfig(Address(SERVER_ADDRESS, SERVER_PORT));
//    clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");
//
//
//    try {
//        HazelcastClient hazelcastClient(clientConfig);
//
//        IMap<int, int> imap = hazelcastClient.getMap<int, int >("testLockMap");
//        boost::hash<boost::thread::id> h;
//        long currentId = h(boost::this_thread::get_id());
//
//        imap.lock(0);
//        std::cout << "pre " << currentId << endl;
//        imap.unlock(0);
//
//        imap.lock(1);
//        std::cout << "critical " << currentId << endl;
//        imap.unlock(1);
//
//        imap.lock(2);
//        std::cout << "out " << currentId << endl;
//        imap.unlock(2);
//
//    } catch (std::exception& e) {
//        std::cout << e.what() << std::endl;
//    }
////    imap.lock(1);
////    imap.unlock(5);
////    imap.isLocked(4);
////    imap.forceUnlock(3);
////    imap.tryLock(3,1000);
//};
//
//void testMapLocksInParallel() {
//
//    try {
//
//        boost::thread t1(testMapLocksInSequential);
//        boost::thread t2(testMapLocksInSequential);
//
//        t1.join();
//        t2.join();
//
//    } catch (std::exception& e) {
//        std::cout << e.what() << std::endl;
//    }
//};
//
//#endif
//
