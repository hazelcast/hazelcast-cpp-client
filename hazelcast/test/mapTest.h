#ifndef MAPTEST
#define MAPTEST

#include "TestMainPortable.h"
#include "testUtil.h"
#include "SimpleMapTest.h"
#include <gtest/gtest.h>

TEST(MapTest, PutGetRemove){
    ClientConfig clientConfig;
    Address address = Address(SERVER_ADDRESS, SERVER_PORT);
    clientConfig.addAddress(address);
    clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");

    try {

        HazelcastClient hazelcastClient(clientConfig);
        IMap<int, TestMainPortable> iMap = hazelcastClient.getMap<int, TestMainPortable >("sancar");
        TestMainPortable mainPortable = getTestMainPortable();
        TestMainPortable empty;

        for (int i = 0; i < 100; i++) {
            iMap.remove(i);
        }

        for (int i = 0; i < 100; i++) {
            TestMainPortable x = mainPortable;
            x.i = i * 10;
            x.p.ii.push_back(i * 100);
            TestMainPortable oldValue = iMap.put(i, x);
            EXPECT_EQ(empty, oldValue);
        }

        for (int i = 0; i < 100; i++) {
            TestMainPortable x = iMap.get(i);
            EXPECT_EQ(i * 10, x.i);
            EXPECT_EQ( i * 100, x.p.ii.at(x.p.ii.size() - 1));
        }

        for (int i = 0; i < 50; i++) {
            TestMainPortable x = iMap.get(i);
            TestMainPortable p = iMap.remove(i);
            EXPECT_EQ(x, p);
        }

        for (int i = 0; i < 50; i++) {
            TestMainPortable x = iMap.get(i);
            EXPECT_EQ(empty, x);
        }

        for (int i = 50; i < 100; i++) {
            TestMainPortable x = mainPortable;
            x.i = i * 20;
            x.p.ii.push_back(i * 200);
            TestMainPortable oldValue = iMap.put(i, x);
            EXPECT_EQ(i * 10, oldValue.i);
            EXPECT_EQ( i * 100, oldValue.p.ii.at(oldValue.p.ii.size() - 1));
        }

        for (int i = 50; i < 100; i++) {
            TestMainPortable x = iMap.remove(i);
            EXPECT_EQ( i * 20, x.i);
            EXPECT_EQ( i * 200, x.p.ii.at(x.p.ii.size() - 1));
        }

    } catch (std::exception& e) {
        std::cout << e.what() << std::endl;
    }
};

int testSpeed() {
    SimpleMapTest s(SERVER_ADDRESS, SERVER_PORT);
    s.run();
    return 0;
};

#endif

