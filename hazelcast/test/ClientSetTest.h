//
//  ClientSetTest.h
//  hazelcast
//
//  Created by Sancar on 16.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_ClientSetTest_h
#define hazelcast_ClientSetTest_h

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"
#include "Iset.h"

class ClientMultiMapTest : public iTestFixture<ClientMultiMapTest> {

public:

    ClientMultiMapTest() {

    };

    void addTests() {
        addTest(&ClientMultiMapTest::testAddAll, "testAddAll");
        addTest(&ClientMultiMapTest::testAddRemove, "testAddRemove");
        addTest(&ClientMultiMapTest::testIterator, "testIterator");
        addTest(&ClientMultiMapTest::testContains, "testContains");
        addTest(&ClientMultiMapTest::removeRetainAll, "removeRetainAll");
        addTest(&ClientMultiMapTest::testListener, "testListener");

    };

    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        set = new ISet(client->getSet("ClientMultiMapTest"));
    };

    void afterClass() {
        delete client;
        delete set;
    };

    void beforeTest() {
        //Empty
    };

    void afterTest() {
        set->clear();
    };


    void testAddAll() {
        /*List l = new ArrayList();
        l.add("item1");
        l.add("item2");
        
        assertTrue(set->addAll(l));
        assertEqual(2, set->size());
        
        assertFalse(set->addAll(l));*/
        assertEqual(2, set->size());

    }

    void testAddRemove() {
        assertTrue(set->add("item1"));
        assertTrue(set->add("item2"));
        assertTrue(set->add("item3"));
        assertEqual(3, set->size());

        assertFalse(set->add("item3"));
        assertEqual(3, set->size());


        assertFalse(set->remove("item4"));
        assertTrue(set->remove("item3"));

    }

    void testIterator() {
        assertTrue(set->add("item1"));
        assertTrue(set->add("item2"));
        assertTrue(set->add("item3"));
        assertTrue(set->add("item4"));

        /* Iterator iter = set->iterator();
         assertTrue(((String)iter.next()).startsWith("item"));
         assertTrue(((String)iter.next()).startsWith("item"));
         assertTrue(((String)iter.next()).startsWith("item"));
         assertTrue(((String)iter.next()).startsWith("item"));
         assertFalse(iter.hasNext());*/
    }

    void testContains() {
        assertTrue(set->add("item1"));
        assertTrue(set->add("item2"));
        assertTrue(set->add("item3"));
        assertTrue(set->add("item4"));

        assertFalse(set->contains("item5"));
        assertTrue(set->contains("item2"));

        /*List l = new ArrayList();
        l.add("item6");
        l.add("item3");
        
        assertFalse(set->containsAll(l));
        assertTrue(set->add("item6"));
        assertTrue(set->containsAll(l));*/
    }


    void removeRetainAll() {
        assertTrue(set->add("item1"));
        assertTrue(set->add("item2"));
        assertTrue(set->add("item3"));
        assertTrue(set->add("item4"));

        /*List l = new ArrayList();
        l.add("item4");
        l.add("item3");
        
        assertTrue(set->removeAll(l));
        assertEqual(2, set->size());
        assertFalse(set->removeAll(l));
        assertEqual(2, set->size());
        
        l.clear();
        l.add("item1");
        l.add("item2");
        assertFalse(set->retainAll(l));
        assertEqual(2, set->size());
        
        l.clear();*/
        assertTrue(set->retainAll(l));
        assertEqual(0, set->size());

    }


    void testListener() {
        //Contain latch
    }

private:
    HazelcastClient *client;
    IList *set;

#endif
