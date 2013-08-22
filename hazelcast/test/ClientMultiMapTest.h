//
//  ClientMultiMapTest.h
//  hazelcast
//
//  Created by Batikan Turkmen on 16.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef hazelcast_ClientMultiMapTest_h
#define hazelcast_ClientMultiMapTest_h

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"
#include "MultiMap.h"

class ClientMultiMapTest: public iTestFixture<ClientMultiMapTest>{
    
public:
    
    ClientMultiMapTest(){
        
    };
    
    void addTests() {
        addTest(&ClientMultiMapTest::testPutGetRemove, "testPutGetRemove");
        addTest(&ClientMultiMapTest::testKeySetEntrySetAndValues, "testKeySetEntrySetAndValues");
        addTest(&ClientMultiMapTest::testContains, "testContains");
        addTest(&ClientMultiMapTest::testListener, "testListener");
        addTest(&ClientMultiMapTest::testLock, "testLock");
        addTest(&ClientMultiMapTest::testLockTtl, "testLockTtl");
        addTest(&ClientMultiMapTest::testTryLock, "testTryLock");
        addTest(&ClientMultiMapTest::testForceUnlock, "testForceUnlock");
        
    };
    
    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        MMap = new MultiMap(client->getMultiMap("ClientMultiMapTest"));
    };
    
    void afterClass() {
        delete client;
        delete MMap;
    };
    
    void testPutGetRemove() {
        assertTrue(MMap->put("key1", "value1"));
        assertTrue(MMap->put("key1", "value2"));
        assertTrue(MMap->put("key1", "value3"));
        
        assertTrue(MMap->put("key2", "value4"));
        assertTrue(MMap->put("key2", "value5"));
        
        assertEqual(3, MMap->valueCount("key1"));
        assertEqual(2, MMap->valueCount("key2"));
        assertEqual(5, MMap->size());
        
       // Collection coll = MMap->get("key1");
        assertEqual(3, coll.size());
        
        coll = MMap->remove("key2");
        assertEqual(2, coll.size());
        assertEqual(0, MMap->valueCount("key2"));
        assertEqual(0, MMap->get("key2").size());
        
        assertFalse(MMap->remove("key1", "value4"));
        assertEqual(3, MMap->size());
        
        assertTrue(MMap->remove("key1", "value2"));
        assertEqual(2, MMap->size());
        
        assertTrue(MMap->remove("key1", "value1"));
        assertEqual(1, MMap->size());
        assertEqual("value3", MMap->get("key1").iterator().next());
    }
    
    
    void testKeySetEntrySetAndValues() {
        assertTrue(MMap->put("key1", "value1"));
        assertTrue(MMap->put("key1", "value2"));
        assertTrue(MMap->put("key1", "value3"));
        
        assertTrue(MMap->put("key2", "value4"));
        assertTrue(MMap->put("key2", "value5"));
        
        
        assertEqual(2, MMap->keySet().size());
        assertEqual(5, MMap->values().size());
        assertEqual(5, MMap->entrySet().size());
    }
    
    
    void testContains() {
        assertTrue(MMap->put("key1", "value1"));
        assertTrue(MMap->put("key1", "value2"));
        assertTrue(MMap->put("key1", "value3"));
        
        assertTrue(MMap->put("key2", "value4"));
        assertTrue(MMap->put("key2", "value5"));
        
        assertFalse(MMap->containsKey("key3"));
        assertTrue(MMap->containsKey("key1"));
        
        assertFalse(MMap->containsValue("value6"));
        assertTrue(MMap->containsValue("value4"));
        
        assertFalse(MMap->containsEntry("key1", "value4"));
        assertFalse(MMap->containsEntry("key2", "value3"));
        assertTrue(MMap->containsEntry("key1", "value1"));
        assertTrue(MMap->containsEntry("key2", "value5"));
    }
   
   void testListener() {
        //This method contain latch
    }

   void testLock() {
       //This method contain latch
    }
    
    void testLockTtl() {
        //This method contain latch
    }
    
   
   void testTryLock() {
       //This method contain latch
    }
    
    void testForceUnlock() {
        //This method contain latch
    }
    
private:
    HazelcastClient *client;
    IList *MMap;

#endif
