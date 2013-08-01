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
    for(int i = 0; i <9; i++){
        
    }
}
void flush(){
    //TODO
}




