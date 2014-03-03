//
// Created by sancar koyunlu on 03/03/14.
//


#ifndef HAZELCAST_TestApp
#define HAZELCAST_TestApp

#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/GroupConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/IMap.h>
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace hazelcast::client;

class TestApp {
public:
    void run(){
        ClientConfig clientConfig;
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        HazelcastClient client(clientConfig);
        IMap<std::string, std::string> m = client.getMap<std::string, std::string>("default");
        boost::shared_ptr<std::string> ptr = m.put("a", "b");
    }
    
private:
};

//int main(){
//    TestApp testApp;
//    testApp.run();
//};


#endif //HAZELCAST_TestApp
