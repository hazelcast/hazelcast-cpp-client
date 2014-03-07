#ifndef HAZELCAST_SIMPLE_MAP_TEST
#define HAZELCAST_SIMPLE_MAP_TEST

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"
#include "InstanceNotActiveException.h"
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
int GET_PERCENTAGE = 30;
int PUT_PERCENTAGE = 30;


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
        std::cerr << "Total = " << total() << ", puts = " << putCount << " , gets = " << getCount << " , removes = " << removeCount << std::endl;
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
            std::cerr << "Operations per Second : " << statsNow.total() / STATS_SECONDS << std::endl;
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
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
        IMap<int, std::vector<char> > map = a->getMap<int, std::vector<char > >("cppDefault");

        std::vector<char> value(VALUE_SIZE);
        bool running = true;
        while (running) {
            int key = rand() % ENTRY_COUNT;
            int operation = (rand() % 100);
            try {
                if (operation < GET_PERCENTAGE) {
                    map.get(key);
                    ++stats.getCount;
                } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                    boost::shared_ptr<std::vector<char> > vector = map.put(key, value);
                    ++stats.putCount;
                } else {
                    map.remove(key);
                    ++stats.removeCount;
                }
            } catch(hazelcast::client::exception::IOException &e) {
                std::cerr << ">hz " << e.what() << std::endl;
            } catch(hazelcast::client::exception::InstanceNotActiveException &e) {
                std::cerr << ">std " << e.what() << std::endl;
            } catch(...) {
                std::cerr << ">unkown exception" << std::endl;
                running = false;
            }

        }
    }

    void run() {
        srand(time(NULL));
        std::cerr << "Starting Test with  " << std::endl;
        std::cerr << "      Thread Count: " << THREAD_COUNT << std::endl;
        std::cerr << "       Entry Count: " << ENTRY_COUNT << std::endl;
        std::cerr << "        Value Size: " << VALUE_SIZE << std::endl;
        std::cerr << "    Get Percentage: " << GET_PERCENTAGE << std::endl;
        std::cerr << "    Put Percentage: " << PUT_PERCENTAGE << std::endl;
        std::cerr << " Remove Percentage: " << (100 - (PUT_PERCENTAGE + GET_PERCENTAGE)) << std::endl;
        ClientConfig clientConfig;
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        clientConfig.addAddress(Address(server_address, server_port)).setAttemptPeriod(10 * 1000);

        boost::thread monitor(printStats);
        HazelcastClient hazelcastClient(clientConfig);

        for (int i = 0; i < THREAD_COUNT; i++) {
            boost::thread t(boost::bind(&SimpleMapTest::op, this, &hazelcastClient));
        }
        monitor.join();
    }
};

#endif
