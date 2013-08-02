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
#include <hazelcast/client/ClientConfig.h>
#include <iTest/iTest.h>


using namespace hazelcast::client::IAtomicLong;
using namespace std;
using namespace iTest;


class ClientAtomiclLong {
    
public:
    
    void addTests() {
        addTest(&ClientAtomiclLong::test, "ClientAtomicLongTest");
    };
    
    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        atom = new IAtomicLong(client->getMap("IAtomicLongTest"));
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
        assert(atom->getAndAdd(2) == 0);
        assert(atom->get() == 0);
        atom->set(5);
        assert(atom->get() == 5);
        assert(atom->getAndAdd(3) == 8);
        assert(!(atom->compareAndSet(7, 4)));
        assert(atom->get() == 8);
        assert(atom->compareAndSet(8, 4));
        assert(atom->get() == 4);
        assert(atom->decrementAndGet() == 3);
        assert(atom->getAndIncrement() == 3);
        assert(atom->getAndSet(9) == 4);
        assert(atom->incrementAndGet() == 10);
    }
    
private:
    HazelcastClient *client;
    IAtomicLong *atom;    
};

#endif
