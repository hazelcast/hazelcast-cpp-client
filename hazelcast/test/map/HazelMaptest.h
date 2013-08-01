//
//  HazelMaptest.cpp
//  hazelcast
//
//  Created by Batikan Turkmen on 30.07.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef HAZEL_MAP_TEST
#define HAZEL_MAP_TEST

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include <hazelcast/client/ClientConfig.h>
#include "iTest.h"
#include <cassert>
#include <vector>

using namespace iTest;
using namespace hazelcast::client;
using namespace std;

class HazelMaptest : public iTestFixture<MapTest> {
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
        iMap = new IMap<string,string>(client->getMap<std::string,std::string>("clientMapTest"));
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
    
    void fillMap();

    
    void fillMap(){
        for (int i=0; i<10; i++){
            string key = "key";
            key += hazelcast::util::to_string(i);
            string value = "value";
            value += hazelcast::util::to_string(i);
            iMap->put(key, value);
        }
    }
    
    void testGet(){
        fillMap();
        for (int i=0; i<10; i++){
            string key = "key";
            key += hazelcast::util::to_string(i);
            string temp = iMap->get(key);
            
            string value = "value";
            value += hazelcast::util::to_string(i);
            assert( temp == value);
        }
    }
    
    void testRemoveAndDelete(){
        fillMap();
        string temp = iMap->remove("key10");
        assert(temp == "");
        iMap->deleteEntry("key9");
        assert(iMap->size() == 9);
        for(int i = 0; i < 9; i++){
            string key = "key";
            key += hazelcast::util::to_string(i);
            string temp = iMap->remove(key);
            string value = "value";
            value += hazelcast::util::to_string(i);
            assert( temp == value);
        }
        assert(iMap->size() == 0);
    }
    
    void testRemoveIfSame(){
        fillMap();
        
        assert(!(iMap->remove("key2", "value")));
        assert(iMap->size() == 10);
        
        assert((iMap->remove("key2", "value1")));
        assert(iMap->size() == 9);
        
    }
    
    void flush(){
        //TODO
    }
    
    void clear() {
        iMap->clear();
    }
    
    void testContains(){
        fillMap();
        
        assert(!(iMap->containsKey("key10")));
        assert(iMap->containsKey("key1"));
        
        assert(!(iMap->containsValue("value10")));
        assert(iMap->containsValue("value1"));
        
    }
    
    void testReplace(){
        string temp = iMap->replace("key1", "value");
        assert(temp == "");
        
        string tempKey = "key1";
        string tempValue = "value1";
        iMap->put(tempKey, tempValue);
        
        assert("value1" == iMap->replace("key1","value2"));
        assert("value2" == iMap->get("key1"));
        
        assert(!(iMap->replace("key1", "value1", "value3")));
        assert("value2" == iMap->get("key1"));
        
        assert((iMap->replace("key1", "value2", "value3")));
        assert("value3" == iMap->get("key1"));
    }
    
    void testPutTtl() {
        
        iMap->put("key1", "value1", 1000);
        string temp = iMap->get("key1");
        assert(temp != "");
        boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
        string temp2 = iMap->get("key1");
        assert(temp2 == "");
    }
    
    void testTryPutRemove(){
        
        // This method contains CountDownLatch
        assert(false);
    }
    
    void testAsyncPutWithTtl(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testIssue537(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testListener(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testTryLock(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testForceUnlock(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testLockTtl2(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testLockTtl(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testLock(){
        
        // This method contains CountDownLatch
        assert(false);
        
    }
    
    void testBasicPredicate(){
        
        fillMap();
        vector<string> tempVector;
        tempVector = iMap->values("this == value1");
        
        vector<string>::iterator it = tempVector.begin();
        assert("value1" == *it);
        
        vector<string> tempVector2;
        tempVector2 =  iMap->keySet("this == value1");
        
        vector<string>::iterator it2 = tempVector2.begin();
        assert("key1" == *it2);
        
        
        std::vector<std::pair<string, string> >  tempVector3;
        tempVector3 = iMap->entrySet("this == value1");
        
        std::vector<std::pair<string, string> > ::iterator it3 = tempVector3.begin();
        assert("key1" == (*it3).first);
        assert("value1" == (*it3).second);
        
    }
    
    void testPutTransient() {
        
        //TODO mapstore
        assert(false);
        
    }
    
    void testSet(){
        
        iMap->set("key1", "value1");
        assert("value1" == iMap->get("key1"));
        
        iMap->set("key1", "value2");
        assert("value2" == iMap->get("key1"));
        
        iMap->set("key1", "value3", 1000);
        assert("value3" == iMap->get("key1"));
        
        boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
        
        assert(iMap->get("key1") == "");
        
    }
    
    void testAsyncGet(){
        
        // This method contains Async functions which is not coded yet
        assert(false);
        
    }
    
    void testAsyncPut(){
        
        // This method contains Async functions which is not coded yet
        assert(false);
        
    }
    
    void testAsyncRemove(){
        
        // This method contains Async functions which is not coded yet
        assert(false);
        
    }
    
    void testValues() {
        
        fillMap();
        vector<string> tempVector;
        tempVector = iMap->values("this == value1");
        assert(1 == tempVector.size());
        
        vector<string>::iterator it = tempVector.begin();
        assert("value1" == *it);
    }
    
    void testGetAllPutAll(){
        
        std::map<string, string> mapTemp;
        
        for (int i = 0; i < 100; i++) {
            mapTemp[hazelcast::util::to_string(i)] = hazelcast::util::to_string(i);
        }
        iMap->putAll(mapTemp);
        assert(iMap->size() == 100);
        for (int i = 0; i < 100; i++) {
            assert(iMap->get(hazelcast::util::to_string(i)) == hazelcast::util::to_string(i));
        }
        
        static IMap<string, string> iMap2 = hazelcastClient.getMap<string, string >("dev");
        
        std::set<string> tempSet;
        tempSet.insert(hazelcast::util::to_string(1));
        tempSet.insert(hazelcast::util::to_string(3));
        
        iMap2.getAll(tempSet);
        
        assert(iMap2.size() == 2);
        assert(iMap2.get(hazelcast::util::to_string(1)) == "1");
        assert(iMap2.get(hazelcast::util::to_string(3)) == "3");
        
    }
    
    void testPutIfAbsent(){
        
        // putIfAbsent method is not coded yet
        assert(false);
        
    }
    
    void testPutIfAbsentTtl(){
        
        // putIfAbsent method is not coded yet
        assert(false);
        
    }
    
    void destroy(){
        
        // waiting for framework
        assert(false);
        
    }
    
private:
    HazelcastClient *client;
    iMap<std::string> *iMap;
};
#endif