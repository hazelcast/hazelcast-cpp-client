#include <iostream>
#include <vector>
#include <signal.h>

#include <hazelcast/client/HazelcastAll.h>
#include <hazelcast/client/query/BetweenPredicate.h>
#include <hazelcast/client/query/QueryConstants.h>
#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/spi/ClientContext.h>

using namespace hazelcast::util;
using namespace hazelcast::client;
using namespace std;

class UpdateEntryProcessor : public serialization::IdentifiedDataSerializable {
public:
    UpdateEntryProcessor(const string &value) : value(new string(value)) {}

    virtual int getFactoryId() const {
        return 66;
    }

    virtual int getClassId() const {
        return 1;
    }

    virtual void writeData(serialization::ObjectDataOutput &writer) const {
        writer.writeUTF(value.get());
    }

    virtual void readData(serialization::ObjectDataInput &reader) {
        throw hazelcast::client::exception::IllegalStateException(
                "UpdateEntryProcessor readData should not be called at client side");
    }

private:
    auto_ptr<string> value;
};

AtomicBoolean isCancelled;

class SoakTestTask : public Runnable {
public:
    SoakTestTask(const IMap<string, string> &map, ILogger &logger) : map(map), logger(logger) {}

    virtual const string getName() const {
        return "SoakTestTask";
    }

    virtual void run() {
        logger.info() << "Thread " << getCurrentThreadId() << " is started.";

        int64_t getCount = 0;
        int64_t putCount = 0;
        int64_t valuesCount = 0;
        int64_t executeOnKeyCount = 0;
        int entryCount = 10000;

        while (!isCancelled) {
            std::ostringstream out;
            int operation = rand() % 100;
            int randomKey = rand() % entryCount;
            out << randomKey;
            string key(out.str());
            try {
                if (operation < 30) {
                    map.get(key);
                    ++getCount;
                } else if (operation < 60) {
                    out.clear();
                    out << rand();
                    map.put(key, out.str());
                    ++putCount;
                } else if (operation < 80) {
                    map.values(query::BetweenPredicate<int>(query::QueryConstants::getValueAttributeName(), 1, 10));
                    ++valuesCount;
                } else {
                    UpdateEntryProcessor processor(out.str());
                    map.executeOnKey<string, UpdateEntryProcessor>(key, processor);
                    ++executeOnKeyCount;
                }

                int64_t totalCount = putCount + getCount + valuesCount + executeOnKeyCount;
                if (totalCount % 10000 == 0) {
                    logger.info() << "Thread " << getCurrentThreadId() << " --> Total:" << totalCount
                                  << ":{getCount:" << getCount << ", putCount:" << putCount << ", valuesCount:"
                                  << valuesCount << ", executeOnKeyCount:" << executeOnKeyCount << "}";
                }
            } catch (std::exception &e) {
                logger.warning() << "Exception occured:  " << e.what();
            }
        }

        int64_t totalCount = putCount + getCount + valuesCount + executeOnKeyCount;
        logger.info() << "Thread " << getCurrentThreadId() << " is ending." << " --> Total:" << totalCount
                      << ":{getCount:" << getCount << ", putCount:" << putCount << ", valuesCount:" << valuesCount
                      << ", executeOnKeyCount:" << executeOnKeyCount << "}";
    }

private:
    IMap<string, string> map;
    ILogger &logger;
};

void signalHandler(int s) {
    cerr << "Caught signal: " << s << endl;
    isCancelled = true;
}

void registerSignalHandler() {
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = signalHandler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);
}

int main(int argc, char *args[]) {
    if (argc != 3) {
        cerr << "USAGE: SoakTest threadCount server_address" << endl;
        return -1;
    }

    const int threadCount = atoi(args[1]);
    const string address = args[2];

    ClientConfig config;
    config.addAddress(Address(address, 5701));

    HazelcastClient hazelcastInstance(config);
    spi::ClientContext context(hazelcastInstance);
    ILogger &logger = context.getLogger();
    IMap<string, string> map = hazelcastInstance.getMap<string, string>("test");

    logger.info() << "Soak test is starting with the following parameters: " << "threadCount = " << threadCount
                  << ", server address = " << address;

    registerSignalHandler();

    vector<std::shared_ptr<Thread> > threads(threadCount);

    for (int i = 0; i < threadCount; i++) {
        std::shared_ptr<Thread> thread(
                new Thread(std::shared_ptr<Runnable>(new SoakTestTask(map, logger)), logger));
        threads[i] = thread;
        thread->start();
    }

    for (int i = 0; i < threadCount; i++) {
        threads[i]->join();
    }
    hazelcastInstance.shutdown();

    return 0;
}

