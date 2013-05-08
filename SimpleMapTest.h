#ifndef HAZELCAST_SIMPLE_MAP_TEST
#define HAZELCAST_SIMPLE_MAP_TEST

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

#include <boost/thread.hpp>
#include <boost/atomic.hpp>
#include <boost/chrono.hpp>

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
        getCount.store(rhs.getCount.load());
        putCount.store(rhs.putCount.load());
        removeCount.store(rhs.removeCount.load());

    };

    Stats getAndReset() {
        Stats newOne(*this);
        putCount.store(0);
        getCount.store(0);
        removeCount.store(0);
        return newOne;
    };
    boost::atomic<long> getCount;
    boost::atomic<long> putCount;
    boost::atomic<long> removeCount;

    void print() {
        std::cout << "Total = " << total() << ", puts = " << putCount.load() << " , gets = " << getCount.load() << " , removes = " << removeCount.load() << std::endl;
    };

    long total() {
        return getCount.load() + putCount.load() + removeCount.load();
    };
} stats;

void printStats() {
    while (true) {
        try {
            boost::chrono::milliseconds duration(STATS_SECONDS * 1000);
            boost::this_thread::sleep_for(duration);
            Stats statsNow = stats.getAndReset();
            statsNow.print();
            std::cout << "Operations per Second : " << statsNow.total() / STATS_SECONDS << std::endl;
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return;
        }
    }
};

class SimpleMapTest {
public:
    std::string server_address;
    std::string server_port;

    SimpleMapTest(std::string address, std::string port) {
        server_address = address;
        server_port = port;
    };

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
        clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");
        clientConfig.setAddress(server_address, server_port);

        boost::thread monitor(printStats);
        try {
            auto_ptr<HazelcastClient> hazelcastClient = HazelcastClient::newHazelcastClient(clientConfig);
            IMap<std::string, vector<char> > map = hazelcastClient->getMap<std::string, vector<char > >("default");
            while (true) {
                char temp[20];
                int size = sprintf(temp, "%d", (int) (rand() % ENTRY_COUNT));
                std::string key(temp, size);

                size = sprintf(temp, "%d", (int) time(NULL));
                std::vector<char> value(temp, temp + size);
                int operation = ((int) (rand() % 100));
                if (operation < GET_PERCENTAGE) {
                    map.get(key);
                    stats.getCount++;
                } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                    map.put(key, value);
                    stats.putCount++;
                } else {
                    map.remove(key);
                    stats.removeCount++;
                }
            }

        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        monitor.join();
    }


};


#endif
