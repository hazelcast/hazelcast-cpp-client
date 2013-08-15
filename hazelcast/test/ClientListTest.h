//
//  ClientListTest.h
//  hazelcast
//
//  Created by Batikan Turkmen on 15.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef hazelcast_ClientListTest_h
#define hazelcast_ClientListTest_h


#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"
#include "IList.h"

using namespace hazelcast::client;
using namespace std;
using namespace iTest;

class ClientListTest: public iTestFixture<ClientListTest>{
    
public:
    
    ClientListTest(){
        
    };
    
    void addTests() {
        //addTest(&ClientLockTest::testLock, "testLock");
        
    };
    
    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        list = new IList(client->getIList("ClientListTest"));
    };
    
    void afterClass() {
        delete client;
        delete list;
    };
    
    void beforeTest() {
        //Empty
    };
    
    void afterTest() {
        list->clear();
    };
    
    void testAddAll() {
        assertTrue(list->add("item1"));
        assertTrue(list->add("item2"));
        list->add(0,"item3");
        assertEqual(3, list->size());
        
        string temp;
        temp = list->set(2, "item4")
        assertEqual("item2",temp);
        
        assertEqual(3, list->size());
        assertEqual("item3", list->get(0));
        assertEqual("item1", list->get(1));
        assertEqual("item4", list->get(2));
        
        assertFalse(list->remove("item2"));
        assertTrue(list->remove("item3"));
        
        string temp1;
        
        temp1 = list.remove(1);
        assertEquals("item4", temp1);
    }
    
    
private:
    HazelcastClient *client;
    IList *list;
};

#endif
