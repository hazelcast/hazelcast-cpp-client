//
//  HazelMaptest.cpp
//  hazelcast
//
//  Created by Batikan Turkmen on 30.07.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#include "TestMainPortable.h"
#include "testUtil.h"
#include "SimpleMapTest.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include <assert.h> 
#include <vector.h>

using namespace hazelcast::client;
using namespace std;

ClientConfig clientConfig;

HazelcastClient hazelcastClient(clientConfig);

static IMap<string, string> iMap = hazelcastClient.getMap<string, string >("dev");

void fillMap();

int test() {
    
    ClientConfig clientConfig;
    Address address = Address(SERVER_ADDRESS, SERVER_PORT);
    clientConfig.addAddress(address);
    clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
    
    return 0;
}


void fillMap(){
    for (int i=0; i<10; i++){
        string key = "key";
        key += hazelcast::util::to_string(i);
        string value = "value";
        value += hazelcast::util::to_string(i);
        iMap.put(key, value);
    }
}

void testGet(){
    fillMap();
    for (int i=0; i<10; i++){
        string key = "key";
        key += hazelcast::util::to_string(i);
        string temp = iMap.get(key);

        string value = "value";
        value += hazelcast::util::to_string(i);
        assert( temp == value);
    }
    
    
}

void testRemoveAndDelete(){
    fillMap();
    string temp = iMap.remove("key10");
    assert(temp == "");
    iMap.deleteEntry("key9");
    assert(iMap.size() == 9);
    for(int i = 0; i < 9; i++){
        string key = "key";
        key += hazelcast::util::to_string(i);
        string temp = iMap.remove(key);
        string value = "value";
        value += hazelcast::util::to_string(i);
        assert( temp == value);
    }
    assert(iMap.size() == 0);
}

void testRemoveIfSame(){
    fillMap();
    
    assert(!(iMap.remove("key2", "value")));
    assert(iMap.size() == 10);
    
    assert((iMap.remove("key2", "value1")));
    assert(iMap.size() == 9);
    
}

void flush(){
    //TODO
}

void clear() {
    iMap.clear();
}

void testContains(){
    fillMap();
    
    assert(!(iMap.containsKey("key10")));
    assert(iMap.containsKey("key1"));

    assert(!(iMap.containsValue("value10")));
    assert(iMap.containsValue("value1"));
    
}

void testReplace(){
    string temp = iMap.replace("key1", "value");
    assert(temp == "");
    
    string tempKey = "key1";
    string tempValue = "value1";
    iMap.put(tempKey, tempValue);
    
    assert("value1" == iMap.replace("key1","value2"));
    assert("value2" == iMap.get("key1"));
    
    assert(!(iMap.replace("key1", "value1", "value3")));
    assert("value2" == iMap.get("key1"));

    assert((iMap.replace("key1", "value2", "value3")));
    assert("value3" == iMap.get("key1"));
}

void testPutTtl() {
    
    iMap.put("key1", "value1", 1000);
    string temp = iMap.get("key1");
    assert(temp != "");    
    boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
    string temp2 = iMap.get("key1");
    assert(temp2 == "");
}

void testTryPutRemove(){
    // This method contains CountDownLatch
}

void testAsyncPutWithTtl(){
    // This method contains CountDownLatch
}

void testIssue537(){
    // This method contains CountDownLatch
}

void testListener(){
    // This method contains CountDownLatch
}

void testBasicPredicate(){
    
    fillMap();
    vector<string> tempVector;
    tempVector = iMap.values("this == value1");
    
    vector<string>::iterator it = tempVector.begin();
    assert("value1" == *it);
    
    vector<string> tempVector2;
    tempVector2 =  iMap.keySet("this == value1");
    
    vector<string>::iterator it2 = tempVector2.begin();
    assert("key1" == *it2);
    
    
    std::vector<std::pair<string, string> >  tempVector3;
    tempVector3 = iMap.entrySet("this == value1");
    
    std::vector<std::pair<string, string> > ::iterator it3 = tempVector3.begin();
    assert("key1" == (*it3).first);
    assert("value1" == (*it3).second);
    
}

