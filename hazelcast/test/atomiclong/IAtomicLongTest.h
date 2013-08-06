//
//  ClientAtomiclLong.h
//  hazelcast
//
//  Created by Batikan Turkmen on 02.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef hazelcast_ClientAtomiclLong_h
#define hazelcast_ClientAtomiclLong_h

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "IAtomicLong.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"

using namespace hazelcast::client;
using namespace std;
using namespace iTest;


class IAtomicLongTest : public iTestFixture<IAtomicLongTest>{
    
public:
    
    IAtomicLongTest(){
        
    };
    
    void addTests() {
        addTest(&IAtomicLongTest::test, "ClientAtomicLongTest");
    };
    
    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        atom = new IAtomicLong(client->getIAtomicLong("IAtomicLongTest"));
    };
    
    void afterClass() {
        delete client;
        delete atom;
    };
    
    void beforeTest() {
        atom->set(0);
    };
    
    void afterTest() {
        atom->set(0);
    };
    
    void test(){
        assertEqual(0, atom->getAndAdd(2));
        assertEqual(2, atom->get());
        atom->set(5);
        assertEqual(5, atom->get());
        assertEqual(8, atom->addAndGet(3));
        assertFalse(atom->compareAndSet(7, 4));
        assertEqual(8, atom->get());
        assertTrue(atom->compareAndSet(8, 4));
        assertEqual(4, atom->get());
        assertEqual(3, atom->decrementAndGet());
        assertEqual(3, atom->getAndIncrement());
        assertEqual(4, atom->getAndSet(9));
        assertEqual(10, atom->incrementAndGet());
    }
    
private:
    HazelcastClient *client;
    IAtomicLong *atom;    
};

#endif
