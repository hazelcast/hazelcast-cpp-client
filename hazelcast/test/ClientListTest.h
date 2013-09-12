//
//  ClientListTest.h
//  hazelcast
//
//  Created by Sancar on 15.08.2013.
//  Copyright (c) 2013 Sancar. All rights reserved.
//

#ifndef hazelcast_ClientListTest_h
#define hazelcast_ClientListTest_h


#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"
#include "Ilist.h"

using namespace hazelcast::client;
using namespace std;
using namespace iTest;

class ClientListTest : public iTestFixture<ClientListTest> {

public:

    ClientListTest() {

    };

    void addTests() {
        addTest(&ClientListTest::testAddAll, "testAddAll");
        addTest(&ClientListTest::testAddSetRemove, "testAddSetRemove");
        addTest(&ClientListTest::testIterator, "testIterator");
        addTest(&ClientListTest::testContains, "testContains");
        addTest(&ClientListTest::removeRetainAll, "removeRetainAll");
        addTest(&ClientListTest::testListener, "testListener");

    };

    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        list = new IList(client->getList("ClientListTest"));
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
        /*List l = new ArrayList();
        l.add("item1");
        l.add("item2");
        
        assertTrue(list->addAll(l));
        assertEqual(2, list->size());
        
        assertTrue(list->addAll(1, l));
        assertEqual(4, list->size());*/
        std::vector<std::string> l;
        l.push_back("item1");
        l.push_back("item2");
        assertTrue(list.addAll(l));


        assertEqual("item1", list->get(0));
        assertEqual("item1", list->get(1));
        assertEqual("item2", list->get(2));
        assertEqual("item2", list->get(3));
    }

    void testAddSetRemove() {
        assertTrue(list->add("item1"));
        assertTrue(list->add("item2"));
        list->add(0, "item3");
        assertEqual(3, list->size());
        string temp = list->set(2, "item4");
        assertEqual("item2", temp);

        assertEqual(3, list->size());
        assertEqual("item3", list->get(0));
        assertEqual("item1", list->get(1));
        assertEqual("item4", list->get(2));

        assertFalse(list->remove("item2"));
        assertTrue(list->remove("item3"));

        temp = list->remove(1);
        assertEqual("item4", temp);

        assertEqual(1, list->size());
        assertEqual("item1", list->get(0));
    }

    void testIndexOf() {
        assertTrue(list->add("item1"));
        assertTrue(list->add("item2"));
        assertTrue(list->add("item1"));
        assertTrue(list->add("item4"));

        assertEqual(-1, list->indexOf("item5"));
        assertEqual(0, list->indexOf("item1"));

        assertEqual(-1, list->lastIndexOf("item6"));
        assertEqual(2, list->lastIndexOf("item1"));
    }

    void testToArray() {
        assertTrue(list->add("item1"));
        assertTrue(list->add("item2"));
        assertTrue(list->add("item1"));
        assertTrue(list->add("item4"));

        std::vector<std::string> ar = list->toArray();

        list->subList(1, 3);

        /*Iterator iter = list->iterator();
        assertEqual("item1",iter.next());
        assertEqual("item2",iter.next());
        assertEqual("item1",iter.next());
        assertEqual("item4",iter.next());
        assertFalse(iter.hasNext());
        
        ListIterator listIterator = list->listIterator(2);
        assertEqual("item1",listIterator.next());
        assertEqual("item4",listIterator.next());
        assertFalse(listIterator.hasNext());
        
        List l = list->subList(1, 3);
        assertEqual(2, l.size());
        assertEqual("item2", l.get(0));
        assertEqual("item1", l.get(1));*/
    }

    void testContains() {
        assertTrue(list->add("item1"));
        assertTrue(list->add("item2"));
        assertTrue(list->add("item1"));
        assertTrue(list->add("item4"));

        assertFalse(list->contains("item3"));
        assertTrue(list->contains("item2"));

        /* List l = new ArrayList();
         l.add("item4");
         l.add("item3");

         assertFalse(list->containsAll(l));
         assertTrue(list->add("item3"));
         assertTrue(list->containsAll(l));*/
    }

    void removeRetainAll() {
        assertTrue(list->add("item1"));
        assertTrue(list->add("item2"));
        assertTrue(list->add("item1"));
        assertTrue(list->add("item4"));

        /*List l = new ArrayList();
        l.add("item4");
        l.add("item3");
        
        assertTrue(list->removeAll(l));
        assertEqual(3, list->size());
        assertFalse(list->removeAll(l));
        assertEqual(3, list->size());
        
        l.clear();
        l.add("item1");
        l.add("item2");
        assertFalse(list->retainAll(l));
        assertEqual(3, list->size());
        
        l.clear();
        assertTrue(list->retainAll(l));
        assertEqual(0, list->size());*/

    }


    void testListener() {
        // Method contains countdownlatch

    }

private:
    HazelcastClient *client;
    IList *list;
};

#endif
