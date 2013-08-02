//
//  MapTest.cpp
//  hazelcast
//
//  Created by Batikan Turkmen on 30.07.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef HAZELCAST_MAP_TEST
#define HAZELCAST_MAP_TEST

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include <hazelcast/client/ClientConfig.h>
#include "iTest.h"
#include <vector>

using namespace iTest;
using namespace hazelcast::client;
using namespace std;

class MapTest : public iTestFixture<MapTest> {
public:
    MapTest() {

    };

    void addTests() {
        addTest(&MapTest::testGet, "testGet");
        addTest(&MapTest::testRemoveAndDelete, "testRemoveAndDelete");
        addTest(&MapTest::testRemoveIfSame, "testRemoveIfSame");
        addTest(&MapTest::testContains, "testContains");
        addTest(&MapTest::testReplace, "testReplace");
        addTest(&MapTest::testPutTtl, "testPutTtl");
        addTest(&MapTest::testTryPutRemove, "testTryPutRemove");
        addTest(&MapTest::testAsyncPutWithTtl, "testAsyncPutWithTtl");
        addTest(&MapTest::testIssue537, "testIssue537");
        addTest(&MapTest::testListener, "testListener");
        addTest(&MapTest::testTryLock, "testTryLock");
        addTest(&MapTest::testForceUnlock, "testForceUnlock");
        addTest(&MapTest::testLockTtl2, "testLockTtl2");
        addTest(&MapTest::testLockTtl, "testLockTtl");
        addTest(&MapTest::testLock, "testLock");
        addTest(&MapTest::testBasicPredicate, "testBasicPredicate");
        addTest(&MapTest::testPutTransient, "testPutTransient");
        addTest(&MapTest::testSet, "testSet");
        addTest(&MapTest::testAsyncGet, "testAsyncGet");
        addTest(&MapTest::testAsyncPut, "testAsyncPut");
        addTest(&MapTest::testAsyncRemove, "testAsyncRemove");
        addTest(&MapTest::testValues, "testValues");
        addTest(&MapTest::testGetAllPutAll, "testGetAllPutAll");
        addTest(&MapTest::testPutIfAbsent, "testPutIfAbsent");
        addTest(&MapTest::testPutIfAbsentTtl, "testPutIfAbsentTtl");

    };

    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        iMap = new IMap<string, string>(client->getMap<std::string, std::string>("clientMapTest"));
    };

    void afterClass() {
        delete client;
        delete iMap;
    };

    void beforeTest() {
        iMap->clear();
    };

    void afterTest() {
        iMap->clear();
    };

    void fillMap() {
        for (int i = 0; i < 10; i++) {
            string key = "key";
            key += hazelcast::util::to_string(i);
            string value = "value";
            value += hazelcast::util::to_string(i);
            iMap->put(key, value);
        }
    }

    void testGet() {
        fillMap();
        for (int i = 0; i < 10; i++) {
            string key = "key";
            key += hazelcast::util::to_string(i);
            string temp = iMap->get(key);

            string value = "value";
            value += hazelcast::util::to_string(i);
            assertEqual( temp , value);
        }
    }

    void testRemoveAndDelete() {
        fillMap();
        string temp = iMap->remove("key10");
        assertEqual(temp , "");
        iMap->deleteEntry("key9");
        assertEqual(iMap->size() , 9);
        for (int i = 0; i < 9; i++) {
            string key = "key";
            key += hazelcast::util::to_string(i);
            string temp = iMap->remove(key);
            string value = "value";
            value += hazelcast::util::to_string(i);
            assertEqual( temp , value);
        }
        assertEqual(iMap->size() , 0);
    }

    void testRemoveIfSame() {
        fillMap();

        assertFalse(iMap->remove("key2", "value"));
        assertEqual(iMap->size() , 10);

        assertTrue((iMap->remove("key2", "value1")));
        assertEqual(iMap->size() , 9);

    }

    void flush() {
        //TODO mapStore
    }

    void clear() {
        iMap->clear();
    }

    void testContains() {
        fillMap();

        assertFalse(iMap->containsKey("key10"));
        assertTrue(iMap->containsKey("key1"));

        assertFalse(iMap->containsValue("value10"));
        assertTrue(iMap->containsValue("value1"));

    }

    void testReplace() {
        string temp = iMap->replace("key1", "value");
        assertEqual(temp , "");

        string tempKey = "key1";
        string tempValue = "value1";
        iMap->put(tempKey, tempValue);

        assertEqual("value1" , iMap->replace("key1", "value2"));
        assertEqual("value2" , iMap->get("key1"));

        assertEqual(false , iMap->replace("key1", "value1", "value3"));
        assertEqual("value2" , iMap->get("key1"));

        assertEqual(true , iMap->replace("key1", "value2", "value3"));
        assertEqual("value3" , iMap->get("key1"));
    }

    void testPutTtl() {

        iMap->put("key1", "value1", 1000);
        string temp = iMap->get("key1");
        assertEqual(temp , "");
        boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
        string temp2 = iMap->get("key1");
        assertEqual(temp2 , "");
    }

    void testTryPutRemove() {

        // This method contains CountDownLatch
        assertTrue(false);
    }

    void testAsyncPutWithTtl() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testIssue537() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testListener() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testTryLock() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testForceUnlock() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testLockTtl2() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testLockTtl() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testLock() {

        // This method contains CountDownLatch
        assertTrue(false);

    }

    void testBasicPredicate() {

        fillMap();
        vector<string> tempVector;
        tempVector = iMap->values("this , value1");

        vector<string>::iterator it = tempVector.begin();
        assertEqual("value1" , *it);

        vector<string> tempVector2;
        tempVector2 = iMap->keySet("this , value1");

        vector<string>::iterator it2 = tempVector2.begin();
        assertEqual("key1" , *it2);


        std::vector<std::pair<string, string> > tempVector3;
        tempVector3 = iMap->entrySet("this , value1");

        std::vector<std::pair<string, string> > ::iterator it3 = tempVector3.begin();
        assertEqual("key1" , (*it3).first);
        assertEqual("value1" , (*it3).second);

    }

    void testPutTransient() {

        //TODO mapstore
        assertTrue(false);

    }

    void testSet() {

        iMap->set("key1", "value1");
        assertEqual("value1" , iMap->get("key1"));

        iMap->set("key1", "value2");
        assertEqual("value2" , iMap->get("key1"));

        iMap->set("key1", "value3", 1000);
        assertEqual("value3" , iMap->get("key1"));

        boost::this_thread::sleep(boost::posix_time::milliseconds(2000));

        assertEqual(iMap->get("key1") , "");

    }

    void testAsyncGet() {

        // This method contains Async functions which is not coded yet
        assertTrue(false);

    }

    void testAsyncPut() {

        // This method contains Async functions which is not coded yet
        assertTrue(false);

    }

    void testAsyncRemove() {

        // This method contains Async functions which is not coded yet
        assertTrue(false);

    }

    void testValues() {

        fillMap();
        vector<string> tempVector;
        tempVector = iMap->values("this , value1");
        assertEqual(1 , tempVector.size());

        vector<string>::iterator it = tempVector.begin();
        assertEqual("value1" , *it);
    }

    void testGetAllPutAll() {

        std::map<string, string> mapTemp;

        for (int i = 0; i < 100; i++) {
            mapTemp[hazelcast::util::to_string(i)] = hazelcast::util::to_string(i);
        }
        iMap->putAll(mapTemp);
        assertEqual(iMap->size() , 100);
        for (int i = 0; i < 100; i++) {
            assertEqual(iMap->get(hazelcast::util::to_string(i)) , hazelcast::util::to_string(i));
        }

        IMap<string, string> iMap2 = client->getMap<string, string >("dev");

        std::set<string> tempSet;
        tempSet.insert(hazelcast::util::to_string(1));
        tempSet.insert(hazelcast::util::to_string(3));

        iMap2.getAll(tempSet);

        assertEqual(iMap2.size() , 2);
        assertEqual(iMap2.get(hazelcast::util::to_string(1)) , "1");
        assertEqual(iMap2.get(hazelcast::util::to_string(3)) , "3");

    }

    void testPutIfAbsent() {

        // putIfAbsent method is not coded yet
        assertTrue(false);

    }

    void testPutIfAbsentTtl() {

        // putIfAbsent method is not coded yet
        assertTrue(false);

    }

    void destroy() {

        // waiting for framework
        assertTrue(false);

    }

private:
    HazelcastClient *client;
    IMap<std::string, std::string> *iMap;
};

#endif