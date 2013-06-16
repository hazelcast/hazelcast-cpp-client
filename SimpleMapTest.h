#ifndef HAZELCAST_SIMPLE_MAP_TEST
#define HAZELCAST_SIMPLE_MAP_TEST

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Thread.h"
#include "hazelcast/util/AtomicInteger.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace hazelcast::client;

int THREAD_COUNT = 1;
int ENTRY_COUNT = 10 * 1000;
int VALUE_SIZE = 1000;
int STATS_SECONDS = 10;
int GET_PERCENTAGE = 0;
int PUT_PERCENTAGE = 100;


class Stats {
public:
    Stats() {
    };

    Stats(const Stats& rhs) {
        Stats newOne;
        getCount = rhs.getCount;
        putCount = rhs.putCount;
        removeCount = rhs.removeCount;
    };

    Stats getAndReset() {
        Stats newOne(*this);
        getCount = 0;
        putCount = 0;
        removeCount = 0;
        return newOne;
    };
    hazelcast::util::AtomicInteger getCount;
    hazelcast::util::AtomicInteger putCount;
    hazelcast::util::AtomicInteger removeCount;

    void print() {
        std::cout << "Total = " << total() << ", puts = " << putCount.get() << " , gets = " << getCount.get() << " , removes = " << removeCount.get() << std::endl;
    };

    int total() {
        return getCount.get() + putCount.get() + removeCount.get();
    };
} stats;

void *printStats(void *) {
    while (true) {
        try {
            sleep(STATS_SECONDS);
            Stats statsNow = stats.getAndReset();
            statsNow.print();
            std::cout << "Operations per Second : " << statsNow.total() / STATS_SECONDS << std::endl;
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return NULL;
        }
    }
};

class SimpleMapTest {
public:
    std::string server_address;
    int server_port;

    SimpleMapTest(std::string address, int port) {
        server_address = address;
        server_port = port;
    };

    void op(IMap<int, vector<char> >& map) {
        char temp[VALUE_SIZE];
        while (true) {
            int key = rand() % ENTRY_COUNT;
            std::vector<char> value(temp, temp + VALUE_SIZE);

            int operation = ((int) (rand() % 100));
            if (operation < GET_PERCENTAGE) {
                map.get(key);
                ++stats.getCount;
            } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                vector<char> vector1 = map.put(key, value);
                ++stats.putCount;
            } else {
                map.remove(key);
                ++stats.removeCount;
            }
        }
    }

    void run() {
        srand(time(NULL));
        std::cout << "Starting Test with " << std::endl;
        std::cout << "      Thread Count: " << THREAD_COUNT << std::endl;
        std::cout << "       Entry Count: " << ENTRY_COUNT << std::endl;
        std::cout << "        Value Size: " << VALUE_SIZE << std::endl;
        std::cout << "    Get Percentage: " << GET_PERCENTAGE << std::endl;
        std::cout << "    Put Percentage: " << PUT_PERCENTAGE << std::endl;
        std::cout << " Remove Percentage: " << (100 - (PUT_PERCENTAGE + GET_PERCENTAGE)) << std::endl;
        ClientConfig clientConfig;
        clientConfig.addAddress(Address(server_address, server_port));
        clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");

        hazelcast::util::Thread monitor(printStats);
        monitor.start();
        try {
            HazelcastClient hazelcastClient(clientConfig);
            IMap<int, vector<char> > map = hazelcastClient.getMap<int, vector<char > >("default");
            op(map);

        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        monitor.join();
    }


};


#endif
