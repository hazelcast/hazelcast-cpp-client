//
//  IdGeneratorTest.h
//  hazelcast
//
//  Created by Batikan Turkmen on 02.08.2013.
//  Copyright (c) 2013 Batikan Turkmen. All rights reserved.
//

#ifndef hazelcast_IdGeneratorTest_h
#define hazelcast_IdGeneratorTest_h


#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/util/Util.h"
#include "IdGenerator.h"
#include "hazelcast/client/ClientConfig.h"
#include "iTest.h"

using namespace hazelcast::client;
using namespace std;
using namespace iTest;


class IdGeneratorTest : public iTestFixture<IdGeneratorTest>{
    
public:
    IdGeneratorTest(){
        
    };
    
    void addTests() {
        addTest(&IdGeneratorTest::testGenerator, "IdGeneratorTest");
    };
    
    void beforeClass() {
        ClientConfig clientConfig;
        clientConfig.addAddress(Address("localhost", 5701));
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        client = new HazelcastClient(clientConfig);
        generator = new IdGenerator(client->getIdGenerator("IdGeneratorTest"));
    };
    
    void afterClass() {
        delete client;
        delete generator;
    };
    
    void beforeTest() {
    };
    
    void afterTest() {
    };
    
    void testGenerator(){
        assertTrue(generator->init(3569));
        assertFalse(generator->init(4569));
        assertEqual(3570, generator->newId());
    };

private:
    HazelcastClient *client;
    IdGenerator *generator;
    
};


#endif
