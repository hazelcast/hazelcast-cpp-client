/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

#include <string>
#include <iomanip>
#include <vector>

#include <boost/date_time/microsec_time_clock.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>

#include <hdr/hdr_histogram.h>

#include <hazelcast/client/HazelcastAll.h>

using namespace hazelcast::client;
using namespace hazelcast::util;

struct ThreadParameters {
    ThreadParameters() : map(NULL),
                         keySetSize(100000),
                         numberOfThreads(40),
                         testDuration(10 * 60 * 1000),
                         operationInterval(1),
                         serverIp("127.0.0.1"),
                         useNearCache(false),
                         outFileName("NearCacheResult.txt"),
                         mapName("NearCachePerformanceMap") {
    }

    IMap<int, int> *map;
    int keySetSize;
    int numberOfThreads;
    int testDuration;      // milliseconds
    int operationInterval; // milliseconds
    std::string serverIp;
    bool useNearCache;
    std::string outFileName;
    std::string mapName;
    std::vector<int64_t> values;
};

std::ostream &operator<<(std::ostream &out, const ThreadParameters &params) {
    out << "Parameters {" << std::endl
    << "\t\t" << "Is near cache used:" << (params.useNearCache ? "yes" : "no") << std::endl
    << "\t\t" << "Histogram output file:" << params.outFileName << std::endl
    << "\t\t" << "Key set size:" << params.keySetSize << std::endl
    << "\t\t" << "Number of threads:" << params.numberOfThreads << std::endl
    << "\t\t" << "Test Duration:" << params.testDuration << " milliseconds" << std::endl
    << "\t\t" << "Operation interval:" << params.operationInterval << " milliseconds" << std::endl
    << "\t\t" << "Connected server ip:" << params.serverIp << std::endl
    << "}" << std::endl;

    return out;
}

class NearCachePerformanceTest {
public:
    NearCachePerformanceTest(ThreadParameters &params) : params(params) { }

    int run() {
        std::cout << "Starting test with the following parameters:" << std::endl << params << std::endl;

        ClientConfig clientConfig;
        if (params.useNearCache) {
            boost::shared_ptr<config::NearCacheConfig<int, int> > nearCacheConfig(
                    new config::NearCacheConfig<int, int>(params.mapName.c_str()));
            clientConfig.addNearCacheConfig<int, int>(nearCacheConfig);
        }

        HazelcastClient client(clientConfig);
        Address serverAddr(params.serverIp, 5701);
        clientConfig.addAddress(serverAddr);

        IMap<int, int> map = client.getMap<int, int>(params.mapName);

        params.map = &map;

        fillMap(params);

        warmup(params);

        if (params.useNearCache) {
            std::cout << "After warmup before test start the near cache stats is:" <<
            map.getLocalMapStats().getNearCacheStats()->toString() << std::endl;
        }

        std::vector<ThreadInfo> allThreads((size_t) params.numberOfThreads);

        for (int i = 0; i < params.numberOfThreads; ++i) {
            allThreads[i].params = params;

            allThreads[i].thread = boost::shared_ptr<Thread>(new Thread(performGetOperations, &allThreads[i].params));
        }

        for (int j = 0; j < params.numberOfThreads; ++j) {
            allThreads[j].thread->join();
        }

        std::cout << "Test finished" << std::endl;

        if (params.useNearCache) {
            std::cout << "Near cache stats after the test is:" <<
            map.getLocalMapStats().getNearCacheStats()->toString() << std::endl;
        }

        return generateHistogram(allThreads);
    }

private:
    struct ThreadInfo {
        boost::shared_ptr<Thread> thread;
        ThreadParameters params;
    };

    int initializeHdr(hdr_histogram *&histogram, int64_t highestTrackableValue) {
        if (0 != hdr_init(
                INT64_C(1), highestTrackableValue, 1,
                &histogram)) {
            std::cerr << "Failed to init hdr histogram" << std::endl;
            return -1;
        }

        return 0;
    }
    
    static void sleepUntil(boost::posix_time::ptime &wakeupTime) {
        boost::posix_time::ptime now = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::time_duration sleepDuration = wakeupTime - now;
        int64_t sleepUSecs = sleepDuration.total_microseconds();
        if (sleepUSecs > 0) {
            usleep((useconds_t) sleepUSecs);
        }
    }

    static void performGets(ThreadParameters &params) {
        int64_t testStart = currentTimeMillis();
        int64_t testEndTime = testStart + params.testDuration;

        boost::posix_time::time_duration configuredLatency = boost::posix_time::milliseconds(params.operationInterval);

        boost::posix_time::ptime expectedStartTime = boost::posix_time::microsec_clock::universal_time();
        while (currentTimeMillis() < testEndTime) {
            int key = rand() % params.keySetSize;

            sleepUntil(expectedStartTime);

            // perform the measured operation
            params.map->get(key);

            boost::posix_time::ptime end = boost::posix_time::microsec_clock::universal_time();

            boost::posix_time::time_duration duration = end - expectedStartTime;
            if (duration.total_microseconds() < 1) {
                std::cerr << "Negative duration:" << duration.total_microseconds() << std::endl;
            }
            params.values.push_back((int64_t) duration.total_microseconds());

            expectedStartTime += configuredLatency;
        }
    }

    void warmup(ThreadParameters &params) {
        for (int i = 0; i < params.keySetSize; ++i) {
            params.map->get(i);
        }
    }

    void fillMap(ThreadParameters &params) {
        for (int i = 0; i < params.keySetSize; ++i) {
            params.map->put(i, i);
        }
    }

    static void performGetOperations(ThreadArgs &args) {
        ThreadParameters *params = (ThreadParameters *) args.arg0;

        performGets(*params);
    }

    int64_t findMaximumValue(const std::vector<ThreadInfo> &allThreadsInfo) {
        int64_t max = 0;
        for (std::vector<ThreadInfo>::const_iterator it = allThreadsInfo.begin();it != allThreadsInfo.end();++it) {
            const std::vector<int64_t> &values = (*it).params.values;
            for (std::vector<int64_t>::const_iterator valIt = values.begin();valIt != values.end();++valIt) {
                if (*valIt > max) {
                    max = *valIt;
                }
            }
        }
        return max;
    }

    int generateHistogram(const std::vector<ThreadInfo> &allThreadsInfo) {
        int64_t highestTrackableValue = findMaximumValue(allThreadsInfo);

        hdr_histogram *latencies = 0;
        int result = initializeHdr(latencies, highestTrackableValue);
        if (result) {
            return result;
        }

        for (int i = 0; i < params.numberOfThreads; ++i) {
            const std::vector<int64_t> &values = allThreadsInfo[i].params.values;
            std::cout << "Thread [" << i << "] recorded " << values.size() << " values" << std::endl;

            for (std::vector<int64_t>::const_iterator it = values.begin(); it != values.end(); ++it) {
                hdr_record_value(latencies, *it);
            }
        }

        FILE *statsFile = std::fopen(params.outFileName.c_str(), "w+");
        if (NULL == statsFile) {
            std::cerr << "Failed to open stats file " << params.outFileName << " for output. Will use stdout." <<
            std::endl;
        }

        hdr_percentiles_print(latencies, (statsFile ? statsFile : stdout), 1, 1.0, CLASSIC);

        if (statsFile) {
            std::fclose(statsFile);
        }

        return 0;
    }

    ThreadParameters &params;
};

std::string getValueString(const std::string &argumentName, const std::string &argument) {
    size_t position = argument.find(argumentName);
    if (position == 0) {
        return argument.substr(position + argumentName.size());
    }

    return "";
}

/**
 * @return -1 if argument is not found or the value is a negative number, otherwise returns the value of the argument
 */
int getPozitifIntArgument(const std::string &argumentName, const std::string &argument, const std::string &usage) {
    std::string value = getValueString(argumentName, argument);
    if (value.size() == 0) {
        return -1;
    }

    int val = std::atoi(value.c_str());
    if (val) {
        if (val < 0) {
            std::cout << argumentName << " can not be negative. Provided value is \'" << value << "\'. " << usage <<
            std::endl;
            return -1;
        }
    }

    return val;
}

/**
 * @return The number of found valid arguments
 */
int parseArguments(int argc, char **argv, struct ThreadParameters &params) {
    const std::string serverIpArg = "--server-ip=";
    const std::string useNearCacheArg = "--use-near-cache";
    const std::string keySetSize = "--key-set-size=";
    const std::string threadCountArg = "--num-threads=";
    const std::string testDurationArg = "--test-duration-in-milliseconds=";
    const std::string operationIntervalArg = "--operation-interval-in-millis=";
    const std::string helpArg = "--help";
    const std::string statsFileArg = "--stats-output-file=";
    std::ostringstream out;
    out << "USAGE: " << argv[0] << serverIpArg << "<value> "
    << serverIpArg << "<value> "
    << useNearCacheArg << " "
    << keySetSize << "<value> "
    << threadCountArg << "<value> "
    << testDurationArg << "<value> "
    << operationIntervalArg << "<value> "
    << helpArg << " "
    << statsFileArg << "<file path>";

    std::string usage = out.str();

    int numFound = 0;
    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];

        if (helpArg == argument) {
            std::cout << usage << std::endl;
            return -1;
        }

        std::string valueString = getValueString(serverIpArg, argument);
        if (valueString.size()) {
            params.serverIp = valueString;
            ++numFound;
        }

        valueString = getValueString(statsFileArg, argument);
        if (valueString.size()) {
            params.outFileName = valueString;
            ++numFound;
        }

        if (argument == useNearCacheArg) {
            params.useNearCache = true;
            ++numFound;
        }

        int val = getPozitifIntArgument(keySetSize, argument, usage);
        if (val > 0) {
            params.keySetSize = val;
        }

        val = getPozitifIntArgument(threadCountArg, argument, usage);
        if (val > 0) {
            params.numberOfThreads = val;
            ++numFound;
        }

        val = getPozitifIntArgument(testDurationArg, argument, usage);
        if (val > 0) {
            params.testDuration = val;
            ++numFound;
        }

        val = getPozitifIntArgument(operationIntervalArg, argument, usage);
        if (val > 0) {
            params.operationInterval = val;
            ++numFound;
        }
    }

    return numFound;
}

int main(int argc, char **argv) {
    ThreadParameters params;

    int result = parseArguments(argc, argv, params);
    if (result < 0) {
        return result;
    }

    NearCachePerformanceTest test(params);

    return test.run();
}



