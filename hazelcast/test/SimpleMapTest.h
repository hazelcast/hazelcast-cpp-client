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

int THREAD_COUNT = 4;
int ENTRY_COUNT = 10 * 1000;
int VALUE_SIZE = 1000;
int STATS_SECONDS = 10;
int GET_PERCENTAGE = 0;
int PUT_PERCENTAGE = 100;


class HAZELCAST_API Stats {
public:
    Stats() {
    };

    Stats(const Stats &rhs) {
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
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        }
    }
};

class HAZELCAST_API SimpleMapTest {
public:
    std::string server_address;
    int server_port;

    SimpleMapTest(std::string address, int port) {
        server_address = address;
        server_port = port;
    };

    void op(HazelcastClient *a) {
        IMap<int, vector<char> > map = a->getMap<int, vector<char > >("default");

        std::vector<char> value(VALUE_SIZE);
        while (true) {
            int key = rand() % ENTRY_COUNT;
            int operation = ((int) (rand() % 100));
            try {
                if (operation < GET_PERCENTAGE) {
                    map.get(key);
                    ++stats.getCount;
                } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                    boost::shared_ptr<vector<char> > vector = map.put(key, value);
                    ++stats.putCount;
                } else {
                    map.remove(key);
                    ++stats.removeCount;
                }
            } catch(hazelcast::client::exception::IException &e) {
                std::cout << ">hz " << e.what() << std::endl;
//                boost::this_thread::sleep(boost::posix_time::seconds(10));
            } catch(std::exception &e) {
                std::cout << ">std " << e.what() << std::endl;
//                boost::this_thread::sleep(boost::posix_time::seconds(10));
            } catch(...) {
                std::cout << "unkown exception" << std::endl;
//                boost::this_thread::sleep(boost::posix_time::seconds(10));
            }

        }
    }

    void run() {
        srand(time(NULL));
        std::cout << "Starting Test with  " << std::endl;
        std::cout << "      Thread Count: " << THREAD_COUNT << std::endl;
        std::cout << "       Entry Count: " << ENTRY_COUNT << std::endl;
        std::cout << "        Value Size: " << VALUE_SIZE << std::endl;
        std::cout << "    Get Percentage: " << GET_PERCENTAGE << std::endl;
        std::cout << "    Put Percentage: " << PUT_PERCENTAGE << std::endl;
        std::cout << " Remove Percentage: " << (100 - (PUT_PERCENTAGE + GET_PERCENTAGE)) << std::endl;
        ClientConfig clientConfig;
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        clientConfig.addAddress(Address(server_address, server_port)).setAttemptPeriod(10 * 1000);
//        clientConfig.setSmart(false);

        try {
            boost::thread monitor(printStats);
            HazelcastClient hazelcastClient(clientConfig);
            for (int i = 0; i < THREAD_COUNT; i++) {
                boost::thread t(boost::bind(&SimpleMapTest::op, this, &hazelcastClient));
            }


            monitor.join();
        } catch (std::exception &e) {
            std::cout << e.what() << std::endl;
        } catch(...) {
            std::cout << "unkown exception simpleMapTest " << std::endl;
        }
    }


};


#endif
