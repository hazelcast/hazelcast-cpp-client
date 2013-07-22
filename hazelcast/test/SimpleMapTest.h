#ifndef HAZELCAST_SIMPLE_MAP_TEST
#define HAZELCAST_SIMPLE_MAP_TEST

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"
#include <boost/thread.hpp>
#include <boost/atomic.hpp>
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
        getCount.exchange(rhs.getCount);
        putCount.exchange(rhs.putCount);
        removeCount.exchange(rhs.removeCount);
    };

    Stats getAndReset() {
        Stats newOne(*this);
        getCount = 0;
        putCount = 0;
        removeCount = 0;
        return newOne;
    };
    boost::atomic<int> getCount;
    boost::atomic<int> putCount;
    boost::atomic<int> removeCount;

    void print() {
        std::cout << "Total = " << total() << ", puts = " << putCount << " , gets = " << getCount << " , removes = " << removeCount << std::endl;
    };

    int total() {
        return getCount + putCount + removeCount;
    };
} stats;

void printStats() {
    while (true) {
        try {
            boost::this_thread::sleep(boost::posix_time::seconds(STATS_SECONDS));
            Stats statsNow = stats.getAndReset();
            statsNow.print();
            std::cout << "Operations per Second : " << statsNow.total() / STATS_SECONDS << std::endl;
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
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
        std::vector<char> value(temp, temp + VALUE_SIZE);
        while (true) {
            int key = rand() % ENTRY_COUNT;
            int operation = ((int) (rand() % 100));
            try{
                if (operation < GET_PERCENTAGE) {
                    map.get(key);
                    ++stats.getCount;
                } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                    vector<char> vector = map.put(key, value);
                    ++stats.putCount;
                } else {
                    map.remove(key);
                    ++stats.removeCount;
                }
            } catch(std::exception& e){
                std::cout << "__";
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
        clientConfig.addAddress(Address(server_address, server_port)).setAttemptPeriod(10 * 1000);
        clientConfig.getGroupConfig().setName("sancar").setPassword("dev-pass");

        try {
            boost::thread monitor(printStats);
            HazelcastClient hazelcastClient(clientConfig);
            IMap<int, vector<char> > map = hazelcastClient.getMap<int, vector<char > >("default");
            op(map);

            monitor.join();
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }


};


#endif
