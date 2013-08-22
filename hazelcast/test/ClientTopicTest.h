//
//  ClientTopicTest.h
//  hazelcast
//
//  Created by Batikan Turkmen on 19.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef hazelcast_ClientTopicTest_h
#define hazelcast_ClientTopicTest_h

#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "HazelcastInstance.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"
#include "ITopic.h"

using namespace hazelcast::client;
using namespace std;
using namespace iTest;

class ClientTopicTest: public iTestFixture<ClientTopicTest>{
    
public:
    
    ClientTopicTest(){
        
    };
    
    void addTests() {
        addTest(&ClientListTest::testListener, "testListener");
        
    };
    
    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        topic = new ITopic(client->getList("ClientListTest"));
    };
    
    void afterClass() {
        delete client;
        delete topic;
    };
    
    void beforeTest() {
        //Empty
    };
    
    void afterTest() {
        //Empty
    };
    
    void testListener(){
        //This method contains latch
    }
    
private:
    HazelcastClient *client;
    ITopic *topic;
};


#endif
