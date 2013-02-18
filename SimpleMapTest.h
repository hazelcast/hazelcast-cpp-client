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
    Stats(){
    };
    Stats(const Stats& rhs){
        Stats newOne;
        gets.store(rhs.gets.load());
        puts.store(rhs.puts.load());
        removes.store(rhs.removes.load());
        
    };
    Stats getAndReset() {
        Stats newOne(*this);
        puts.store(0);
        gets.store(0);
        removes.store(0);
        return newOne;
    };
    boost::atomic<long> gets;
    boost::atomic<long> puts;
    boost::atomic<long> removes;

    void print() {
        std::cout << "Total = " << total() << ", puts = " << puts.load() << " , gets = " << gets.load() << " , removes = " << removes.load() << std::endl;
    };

    long total() {
        return gets.load() + puts.load() + removes.load();
    };
} stats;

void printStats() {
    while (true) {
        try {
            boost::chrono::milliseconds dura(STATS_SECONDS * 1000);
            boost::this_thread::sleep_for(dura);
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
    SimpleMapTest(std::string address) {
        server_address = address;
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
        clientConfig.setAddress(server_address);

        boost::thread monitor(printStats);
        try {
            auto_ptr<HazelcastClient> hazelcastClient = HazelcastClient::newHazelcastClient(clientConfig);
            IMap<std::string, vector<char> > map = hazelcastClient->getMap<std::string, vector<char > > ("default");
            while (true) {
                char temp[20];
                int size = sprintf(temp,"%d",(int) (rand() % ENTRY_COUNT));
                std::string key(temp,size);
                
                size = sprintf(temp,"%d", (int)time(NULL));
                std::vector<char> value(temp, temp + size);
                int operation = ((int) (rand() % 100));
                if (operation < GET_PERCENTAGE) {
                    map.get(key);
                    stats.gets++;
                } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                    map.put(key, value);
                    stats.puts++;
                } else {
                    map.remove(key);
                    stats.removes++;
                }
            }

        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        monitor.join();
    }




};



#endif
