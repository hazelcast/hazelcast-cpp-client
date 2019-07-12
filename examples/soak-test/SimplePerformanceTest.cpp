#include "hazelcast/client/HazelcastClient.h"

#include "hazelcast/client/ClientConfig.h"
#include "hazelcast/util/Atomic.h"

using namespace std;

class SimpleMapTest {
public:
    void start(const char *serverAddr, int putPercent, int getPercent, int removePercent) {
        std::cerr << "Starting Test with  " << std::endl;
        std::cerr << "      Thread Count: " << THREAD_COUNT << std::endl;
        std::cerr << "       Entry Count: " << ENTRY_COUNT << std::endl;
        std::cerr << "        Value Size: " << VALUE_SIZE << std::endl;
        std::cerr << "    Get Percentage: " << getPercent << std::endl;
        std::cerr << "    Put Percentage: " << putPercent << std::endl;
        std::cerr << " Remove Percentage: " << removePercent << std::endl;
        ClientConfig clientConfig;
        clientConfig.addAddress(Address(serverAddr, 5701));

        Stats stats;
        boost::shared_ptr<hazelcast::util::ILogger> logger(
                new hazelcast::util::ILogger("SimpleMapTest", "SimpleMapTest", "testversion", config::LoggerConfig()));
        hazelcast::util::Thread monitor(boost::shared_ptr<hazelcast::util::Runnable>(new StatsPrinterTask(stats)),
                                        *logger);

        HazelcastClient hazelcastClient(clientConfig);

        IMap<int, std::vector<char> > map = hazelcastClient.getMap<int, std::vector<char> >("cppDefault");

        std::vector<boost::shared_ptr<hazelcast::util::Thread> > threads;

        for (int i = 0; i < THREAD_COUNT; i++) {
            boost::shared_ptr<hazelcast::util::Thread> thread = boost::shared_ptr<hazelcast::util::Thread>(
                    new hazelcast::util::Thread(
                            boost::shared_ptr<hazelcast::util::Runnable>(
                                    new Task(stats, map, putPercent, getPercent, removePercent)), *logger));
            thread->start();
            threads.push_back(thread);
        }

        monitor.start();
        monitor.join();
    }

protected:
    static const int THREAD_COUNT = 40;
    static const int ENTRY_COUNT = 10000;
    static const int VALUE_SIZE = 10;
    static const int STATS_SECONDS = 10;

    class Stats {
    public:
        Stats() : getCount(0), putCount(0), removeCount(0) {
        }

        Stats(const Stats &rhs) : getCount(rhs.getCount.get()), putCount(rhs.putCount.get()),
                                  removeCount(rhs.removeCount.get()) {
        }

        Stats(const Stats &rhs, int putPercent, int getPercent, int removePercent) : getCount(
                const_cast<Stats &>(rhs).getCount.get()),
                                                                                     putCount(
                                                                                             const_cast<Stats &>(rhs).putCount.get()),
                                                                                     removeCount(
                                                                                             const_cast<Stats &>(rhs).removeCount.get()),
                                                                                     putPercent(putPercent),
                                                                                     getPercent(getPercent),
                                                                                     removePercent(removePercent) {
        }

        Stats getAndReset() {
            Stats newOne(*this);
            getCount = 0;
            putCount = 0;
            removeCount = 0;
            return newOne;
        }

        mutable hazelcast::util::Atomic<int64_t> getCount;
        mutable hazelcast::util::Atomic<int64_t> putCount;
        mutable hazelcast::util::Atomic<int64_t> removeCount;

        int putPercent;
        int getPercent;
        int removePercent;

        void print() const {
            std::cerr << "Total = " << total() << ", puts = " << putCount << " , gets = " << getCount << " , removes = "
                      << removeCount << std::endl;
        }

        int total() const {
            return (int) getCount + (int) putCount + (int) removeCount;
        }
    };

    class StatsPrinterTask : public hazelcast::util::Runnable {
    public:
        StatsPrinterTask(Stats &stats) : stats(stats) {}

        virtual void run() {
            while (true) {
                try {
                    hazelcast::util::sleep((unsigned int) STATS_SECONDS);
                    const Stats statsNow = stats.getAndReset();
                    statsNow.print();
                    std::cerr << "Operations per Second : " << statsNow.total() / STATS_SECONDS << std::endl;
                } catch (std::exception &e) {
                    std::cerr << e.what() << std::endl;
                }
            }
        }

        virtual const std::string getName() const {
            return "StatPrinterTask";
        }

    private:
        Stats &stats;
    };

    class Task : public hazelcast::util::Runnable {
    public:
        Task(Stats &stats, IMap<int, std::vector<char> > &map, int putPercent, int getPercent, int removePercent)
                : stats(stats), map(map),
                  logger(new hazelcast::util::ILogger("SimpleMapTest",
                                                      "SimpleMapTest",
                                                      "testversion",
                                                      config::LoggerConfig())),
                  putPercent(putPercent),
                  getPercent(getPercent),
                  removePercent(removePercent) {}

        virtual void run() {
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
                    if (operation < getPercent) {
                        map.get(key);
                        ++getCount;
                    } else if (operation < putPercent) {
                        boost::shared_ptr<std::vector<char> > vector = map.put(key, value);
                        ++putCount;
                    } else {
                        map.remove(key);
                        ++removeCount;
                    }
                    updateStats(updateIntervalCount, getCount, putCount, removeCount);
                } catch (hazelcast::client::exception::IOException &e) {
                    logger->warning(
                            std::string("[SimpleMapTest IOException] ") + e.what());
                } catch (hazelcast::client::exception::HazelcastClientNotActiveException &e) {
                    logger->warning(
                            std::string("[SimpleMapTest::run] ") + e.what());
                } catch (hazelcast::client::exception::IException &e) {
                    logger->warning(
                            std::string("[SimpleMapTest:run] ") + e.what());
                } catch (...) {
                    logger->warning("[SimpleMapTest:run] unknown exception!");
                    running = false;
                    throw;
                }
            }
        }

        virtual const std::string getName() const {
            return "SimpleMapTest Task";
        }

    private:
        int putPercent;
        int getPercent;
        int removePercent;

        void updateStats(int updateIntervalCount, int &getCount, int &putCount, int &removeCount) const {
            if ((getCount + putCount + removeCount) % updateIntervalCount == 0) {
                int64_t current = stats.getCount;
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

        Stats &stats;
        IMap<int, std::vector<char> > &map;
        boost::shared_ptr<hazelcast::util::ILogger> logger;
    };


};

int main(int argc, char *args[]) {
    if (argc != 5) {
        cerr << "USAGE: SoakTest server_address putPercent getPercent removePercent" << endl;
        return -1;
    }

    SimpleMapTest test;
    test.start(args[1], atoi(args[2]), atoi(args[3]), atoi(args[4]));

    return 0;
}

