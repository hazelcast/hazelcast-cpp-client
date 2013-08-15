//
//  ClientLockTest.h
//  hazelcast
//
//  Created by Batikan Turkmen on 15.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef hazelcast_ClientLockTest_h
#define hazelcast_ClientLockTest_h

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"

using namespace hazelcast::client;
using namespace std;
using namespace iTest;

class ClientLockTest: public iTestFixture<ClientLockTest>{
    
public:
    
    ClientLockTest(){
        
    };
    
    void addTests() {
        addTest(&ClientLockTest::testLock, "testLock");
        addTest(&ClientLockTest::testLockTtl, "testLockTtl");
        addTest(&ClientLockTest::testTryLock, "testTryLock");
        addTest(&ClientLockTest::testForceUnlock, "testForceUnlock");
        addTest(&ClientLockTest::testStats, "testStats");
    };
    
    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        lock = new ILock(client->getILock("ClientLockTest"));
    };
    
    void afterClass() {
        delete client;
        delete lock;
    };
    
    void beforeTest() {
        //Empty
    };
    
    void afterTest() {
        lock->forceUnlock();
    };
    
    void testLock(){
        //This method contains countdownlatch
    }
    void testLockTtl(){
        //This method contains countdownlatch
    }
    void testTryLock(){
        //This method contains countdownlatch
    }
    void testForceUnlock(){
        //This method contains countdownlatch
    }
    void testStats(){
        //This method contains countdownlatch
    }

    
private:
    HazelcastClient *client;
    ILock *lock;
};


#endif
