/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAZELCAST_SIMPLE_MAP_TEST
#define HAZELCAST_SIMPLE_MAP_TEST

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/client/GroupConfig.h"
#include "hazelcast/client/HazelcastClient.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/client/exception/InstanceNotActiveException.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/Util.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace hazelcast::client;

int THREAD_COUNT = 40;
int ENTRY_COUNT = 10000;
int VALUE_SIZE = 10;
int STATS_SECONDS = 10;
int GET_PERCENTAGE = 40;
int PUT_PERCENTAGE = 40;


class Stats {
public:
    Stats() {
    };

    Stats(const Stats &rhs) {
        Stats newOne;
        getCount = (int)rhs.getCount;
        putCount = (int)rhs.putCount;
        removeCount = (int)rhs.removeCount;
    };

    Stats getAndReset() {
        Stats newOne(*this);
        getCount = 0;
        putCount = 0;
        removeCount = 0;
        return newOne;
    };
    mutable hazelcast::util::AtomicInt getCount;
    mutable hazelcast::util::AtomicInt putCount;
    mutable hazelcast::util::AtomicInt removeCount;

    void print() const{
        std::cerr << "Total = " << total() << ", puts = " << putCount << " , gets = " << getCount << " , removes = " << removeCount << std::endl;
    };

    int total() const{
        return (int) getCount + (int) putCount + (int) removeCount;
    };
} stats;

void printStats(hazelcast::util::ThreadArgs &args) {
    while (true) {
        try {
            hazelcast::util::sleep((unsigned int)STATS_SECONDS);
            const Stats statsNow = stats.getAndReset();
            statsNow.print();
            std::cerr << "Operations per Second : " << statsNow.total() / STATS_SECONDS << std::endl;
        } catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
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

    static void staticOp(hazelcast::util::ThreadArgs &args) {
        SimpleMapTest *simpleMapTest = (SimpleMapTest *) args.arg0;
        HazelcastClient *hazelcastClient = (HazelcastClient *) args.arg1;
        simpleMapTest->op(hazelcastClient);
    }

    void op(HazelcastClient *a) {
        IMap<int, std::vector<char> > map = a->getMap<int, std::vector<char > >("cppDefault");

        std::vector<char> value(VALUE_SIZE);
        bool running = true;
        int getCount = 0;
        int putCount = 0;
        int removeCount = 0;

        int updateIntervalCount = 1000;
        while (running) {
            int key = rand() % ENTRY_COUNT;
            int operation = (rand() % 100);
            try {
                if (operation < GET_PERCENTAGE) {
                    map.get(key);
                    ++getCount;
                } else if (operation < GET_PERCENTAGE + PUT_PERCENTAGE) {
                    boost::shared_ptr<std::vector<char> > vector = map.put(key, value);
                    ++putCount;
                } else {
                    map.remove(key);
                    ++removeCount;
                }
                updateStats(updateIntervalCount, getCount, putCount, removeCount);
            } catch(hazelcast::client::exception::IOException &e) {
                hazelcast::util::ILogger::getLogger().warning(std::string("[SimpleMapTest IOException] ") + e.what());
            } catch(hazelcast::client::exception::InstanceNotActiveException &e) {
                hazelcast::util::ILogger::getLogger().warning(std::string("[SimpleMapTest InstanceNotActiveException] ") + e.what());
            } catch(hazelcast::client::exception::IException &e) {
                hazelcast::util::ILogger::getLogger().warning(std::string("[SimpleMapTest IException] ") + e.what());
            } catch(...) {
                hazelcast::util::ILogger::getLogger().warning("[SimpleMapTest unknown exception]");
                running = false;
                throw;
            }
        }
    }

    void updateStats(int updateIntervalCount, int &getCount, int &putCount, int &removeCount) const {
        if ((getCount + putCount + removeCount) % updateIntervalCount == 0) {
                        int current = stats.getCount;
                        stats.getCount = current + getCount;
                        getCount = 0;

                        current = stats.putCount;
                        stats.putCount = current + putCount;
                        putCount = 0;

                        current = stats.removeCount;
                        stats.removeCount = current + removeCount;
                        removeCount = 0;
                    }
    }

    void run() {
        std::cerr << "Starting Test with  " << std::endl;
        std::cerr << "      Thread Count: " << THREAD_COUNT << std::endl;
        std::cerr << "       Entry Count: " << ENTRY_COUNT << std::endl;
        std::cerr << "        Value Size: " << VALUE_SIZE << std::endl;
        std::cerr << "    Get Percentage: " << GET_PERCENTAGE << std::endl;
        std::cerr << "    Put Percentage: " << PUT_PERCENTAGE << std::endl;
        std::cerr << " Remove Percentage: " << (100 - (PUT_PERCENTAGE + GET_PERCENTAGE)) << std::endl;
        ClientConfig clientConfig;
        clientConfig.setProperty(ClientProperties::PROP_HEARTBEAT_TIMEOUT, "10");
        clientConfig.getGroupConfig().setName("dev").setPassword("dev-pass");
        clientConfig.addAddress(Address(server_address, server_port)).setAttemptPeriod(10 * 1000);
        clientConfig.setLogLevel(FINEST);
        hazelcast::util::Thread monitor(printStats);
        HazelcastClient hazelcastClient(clientConfig);

        std::vector< hazelcast::util::Thread * > threads(THREAD_COUNT);

        for (int i = 0; i < THREAD_COUNT; i++) {
            threads[i] = new hazelcast::util::Thread(&SimpleMapTest::staticOp, this, &hazelcastClient);
        }
        monitor.join();

        for (int i = 0; i < THREAD_COUNT; i++) {
            delete threads[i];
        }
    }
};

#endif

