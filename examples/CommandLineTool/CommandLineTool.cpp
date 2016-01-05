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
//
// Created by sancar koyunlu on 03/03/14.
//

#include "hazelcast/client/HazelcastAll.h"
#include <ctime>
using namespace hazelcast::client;

//utilites
bool silent;

template<typename T>
std::string toString(const T &value) {
    std::stringstream s;
    s << value;
    return s.str();
}

template<typename T>
T toValue(const std::string &str) {
    std::stringstream s(str);
    T value;
    s >> value;
    return value;
}

struct printer {
    template<typename T>
    printer &operator <<(const T &obj) {
        if (silent) {
            return *this;
        }
        std::cout << obj;
        return *this;
    }
};

struct printer mout;

struct printerSharedPtr {
    template<typename T>
    printer &operator <<(const boost::shared_ptr<T> &obj) {
        if (silent) {
            return mout;
        }
        if (obj.get() == NULL) {
            std::cout << "NULL";
        } else {
            std::cout << *obj;
        }
        return mout;
    }
};

static struct printerSharedPtr moutPtr;

bool startsWith(const std::string &str, const std::string &item) {
    unsigned long foundPosition = str.find(item, 0);
    if (foundPosition == 0) {
        return true;
    }
    return false;
}

bool endsWith(const std::string &str, const std::string &item) {
    unsigned long foundPosition = str.rfind(item);
    if (foundPosition == str.size() - item.size()) {
        return true;
    }
    return false;
}

bool contains(const std::string &str, const std::string &item) {
    unsigned long foundPosition = str.find(item, 0);
    if (foundPosition == std::string::npos) {
        return false;
    }
    return true;
}

bool equals(const std::string &rhs, const std::string &lhs) {
    return rhs.compare(lhs) == 0;
}

std::vector<std::string> split(std::string input, char anchor) {
    std::vector<std::string> out;
    unsigned long found = input.find_first_of(anchor);
    while (found != std::string::npos) {
        out.push_back(input.substr(0, found));
        input = input.substr(found + 1, input.size() - found);
        found = input.find_first_of(anchor);
    }
    out.push_back(input);
    return out;

}
//: public  EntryListener, ItemListener, MessageListener

class CommandLineTool {
private:
    static const int LOAD_EXECUTORS_COUNT = 16;
    static const int ONE_KB = 1024;
    static const int ONE_THOUSAND = 1000;
    static const int ONE_HUNDRED = 100;
    static const int ONE_MINUTE = 60;
    static const int ONE_HOUR = 3600;

    HazelcastClient &hazelcast;

//ITopic<std::string> topic;

//MultiMap<std::string, std::string> multiMap;

//ISet<std::string> set;

//IList<std::string> list;

//IAtomicLong atomicNumber;

    std::string _namespace;

    bool echo;

//LineReader lineReader;

    bool running;
public:
    CommandLineTool(HazelcastClient &hazelcast)
    : hazelcast(hazelcast)
    , _namespace("default")
    , echo(false) {
        silent = false;
    }

    IQueue<std::string> getQueue() {
        return hazelcast.getQueue<std::string>(_namespace);
    }

//ITopic<std::string> getTopic() {
//    topic = hazelcast.getTopic(_namespace);
//    return topic;
//}

    IMap<std::string, std::string> getMap() {
        return hazelcast.getMap<std::string, std::string >(_namespace);
    }
//
//MultiMap<std::string, std::string> getMultiMap() {
//    multiMap = hazelcast.getMultiMap(_namespace);
//    return multiMap;
//}
//
//
//IAtomicLong getAtomicNumber() {
//    atomicNumber = hazelcast.getIAtomicLong(_namespace);
//    return atomicNumber;
//}
//
//ISet<std::string> getSet() {
//    set = hazelcast.getSet(_namespace);
//    return set;
//}
//
//IList<std::string> getList() {
//    list = hazelcast.getList(_namespace);
//    return list;
//}


    void start() {
        getMap().size();
//    getList().size();
//    getSet().size();
        getQueue().size();
//    getMultiMap().size();
//    hazelcast.getExecutorService("default").getLocalExecutorStats();
//    for (int k = 1; k <= LOAD_EXECUTORS_COUNT; k++) {
//        hazelcast.getExecutorService("e" + k).getLocalExecutorStats();
//    }

        running = true;
        while (running) {
            mout << "hazelcast[" << _namespace << "] > ";
            try {
                std::string line;
                std::getline(std::cin, line);
                handleCommand(line);
            } catch (std::exception &e) {
                mout << e.what() << "\n";
            }
        }
    }


    /**
    * Handle a command
    *
    * @param commandInputted
    */
    void handleCommand(std::string commandInputted) {
        std::string command = commandInputted;

//        if ( contains(command, "__")) {
//            _namespace = command.split("__")[0];
//            command = command.substring(command.indexOf("__") + 2);
//        }

        if (echo) {
            handleEcho(command);
        }

        ;
        if (command.length() == 0 || startsWith(command, "//")) {
            return;
        }


        std::vector<std::string> args = split(command, ' ');
        std::string first = args[0];

        if (startsWith(first, "help")) {
            handleHelp(args);
//        } else if (first.startsWith("#") && first.length() > 1) {
//            int repeat = toValue<int>(first.substring(1));
//            long t0 = time(NULL);
//            for (int i = 0; i < repeat; i++) {
//                handleCommand(command.substring(first.length()).replaceAll("\\$i", "" + i));
//            }
//            mout << "ops/s = " << repeat * ONE_THOUSAND / (time(NULL) - t0) << "\n";
//        } else if (first.startsWith("&") && first.length() > 1) {
//            const int fork = toValue<int>(first.substring(1));
//            ExecutorService pool = Executors.newFixedThreadPool(fork);
//            const std::string threadCommand = command.substring(first.length());
//            for (int i = 0; i < fork; i++) {
//                const int threadID = i;
//                pool.submit(new Runnable()
//                {
//                    void run() {
//                        std::string command = threadCommand;
//                        std::string *threadArgs = command.replaceAll("\\$t", "" + threadID).trim()
//                                .split(" ");
//                        // TODO &t #4 m.putmany x k
//                        if ("m.putmany".equals(threadArgs[0])
//                                || "m.removemany".equals(threadArgs[0])) {
//                            if (threadArgs.length < 4) {
//                                command += " " + toValue<int>(threadArgs[1]) * threadID;
//                            }
//                        }
//                        handleCommand(command);
//                    }
//                });
//            }
//            pool.shutdown();
//            try {
//                // wait 1h
//                pool.awaitTermination(ONE_HOUR);
//            } catch (Exception e) {
//                std::cout << e.what() << "\n";
//            }
//        } else if (first.startsWith("@")) {
//            handleAt(first);
//        } else if (command.indexOf(';') != -1) {
//            handleColon(command);
        } else if (equals(first, "silent")) {
            silent = toValue<bool>(args[1]);
        } else if (equals(first, "shutdown")) {
            hazelcast.shutdown();
        } else if (equals(first, "echo")) {
            echo = toValue<bool>(args[1]);
            mout << "echo: " << echo << "\n";
        } else if (equals(first, "ns")) {
            handleNamespace(args);
        } else if (equals(first, "who")) {
            handleWho();
//        } else if (first.contains("ock") && !first.contains(".")) {
//            handleLock(args);
        } else if (contains(first, ".size")) {
            handleSize(args);
        } else if (contains(first, ".clear")) {
            handleClear(args);
        } else if (contains(first, ".destroy")) {
            handleDestroy(args);
        } else if (contains(first, ".iterator")) {
            handleIterator(args);
        } else if (contains(first, ".contains")) {
            handleContains(args);
//        } else if ( contains(first, ".stats")) {
//            handStats(args);
//        } else if( equals(first, "t.publish")) {
//            handleTopicPublish(args);
        } else if (equals(first, "q.offer")) {
            handleQOffer(args);
        } else if (equals(first, "q.take")) {
            handleQTake(args);
        } else if (equals(first, "q.poll")) {
            handleQPoll(args);
        } else if (equals(first, "q.peek")) {
            handleQPeek(args);
        } else if (equals(first, "q.capacity")) {
            handleQCapacity(args);
        } else if (equals(first, "q.offermany")) {
            handleQOfferMany(args);
        } else if (equals(first, "q.pollmany")) {
            handleQPollMany(args);
//        } else if( equals(first, "s.add")) {
//            handleSetAdd(args);
//        } else if( equals(first, "s.remove")) {
//            handleSetRemove(args);
//        } else if( equals(first, "s.addmany")) {
//            handleSetAddMany(args);
//        } else if( equals(first, "s.removemany")) {
//            handleSetRemoveMany(args);
        } else if (equals("m.replace", first)) {
            handleMapReplace(args);
        } else if (equals("m.putIfAbsent", first)) {
            handleMapPutIfAbsent(args);
//        } else if( equals("m.putAsync", first)) {
//            handleMapPutAsync(args);
//        } else if( equals("m.getAsync", first)) {
//            handleMapGetAsync(args);
        } else if (equals("m.put", first)) {
            handleMapPut(args);
        } else if (equals("m.get", first)) {
            handleMapGet(args);
        } else if (equals("m.getMapEntry", first)) {
            handleMapGetMapEntry(args);
        } else if (equals("m.remove", first)) {
            handleMapRemove(args);
        } else if (equals("m.evict", first)) {
            handleMapEvict(args);
        } else if (equals(first, "m.putmany") || equals(first, "m.putAll")) {
            handleMapPutMany(args);
        } else if (equals("m.getmany", first)) {
            handleMapGetMany(args);
        } else if (equals("m.removemany", first)) {
            handleMapRemoveMany(args);
        } else if (equals("m.keys", first)) {
            handleMapKeys();
        } else if (equals("m.values", first)) {
            handleMapValues();
        } else if (equals("m.entries", first)) {
            handleMapEntries();
        } else if (equals("m.lock", first)) {
            handleMapLock(args);
        } else if (equals("m.tryLock", first)) {
            handleMapTryLock(args);
        } else if (equals("m.unlock", first)) {
            handleMapUnlock(args);
//        } else if ( contains(".addListener"), first)) {
//            handleAddListener(args);
//        } else if( equals("m.removeMapListener", first)) {
//            handleRemoveListener(args);
        } else if (equals("m.unlock", first)) {
            handleMapUnlock(args);
//        } else if( equals("mm.put", first)) {
//            handleMultiMapPut(args);
//        } else if( equals("mm.get", first)) {
//            handleMultiMapGet(args);
//        } else if( equals("mm.remove", first)) {
//            handleMultiMapRemove(args);
//        } else if( equals("mm.keys", command)) {
//            handleMultiMapKeys();
//        } else if( equals("mm.values", command)) {
//            handleMultiMapValues();
//        } else if( equals("mm.entries", command)) {
//            handleMultiMapEntries();
//        } else if( equals("mm.lock", first)) {
//            handleMultiMapLock(args);
//        } else if( equals("mm.tryLock", first)) {
//            handleMultiMapTryLock(args);
//        } else if( equals("mm.unlock", first)) {
//            handleMultiMapUnlock(args);
//        } else if( equals("l.add", first)) {
//            handleListAdd(args);
//        } else if( equals("l.set", first)) {
//            handleListSet(args);
//        } else if( equals(first, "l.addmany")) {
//            handleListAddMany(args);
//        } else if( equals("l.remove", first)) {
//            handleListRemove(args);
//        } else if( equals("l.contains", first)) {
//            handleListContains(args);
//        } else if( equals(first, "a.get")) {
//            handleAtomicNumberGet(args);
//        } else if( equals(first, "a.set")) {
//            handleAtomicNumberSet(args);
//        } else if( equals(first, "a.inc")) {
//            handleAtomicNumberInc(args);
//        } else if( equals(first, "a.dec")) {
//            handleAtomicNumberDec(args);
//        } else if( equals("execute", first)) {
//            execute(args);
//        } else if( equals("partitions", first)) {
//            handlePartitions(args);
//        } else if( equals("txn", first)) {
//            hazelcast.getTransaction().begin();
//        } else if( equals("commit", first)) {
//            hazelcast.getTransaction().commit();
//        } else if( equals("rollback", first)) {
//            hazelcast.getTransaction().rollback();
//        } else if( equals("executeOnKey", first)) {
//            executeOnKey(args);
//        } else if( equals("executeOnMember", first)) {
//            executeOnMember(args);
//        } else if( equals("executeOnMembers", first)) {
//            executeOnMembers(args);
            // } else if (first.equals("longOther")
            // || first.equals("executeLongOther")) {
            //     executeLongTaskOnOtherMember(args);
            //} else if (first.equals("long")
            // || first.equals("executeLong")) {
            //    executeLong(args);
//        } else if( equals("instances", first)) {
//            handleInstances(args);
        } else if (equals(first, "quit") || equals(first, "exit")) {
            exit(0);
//        } else if (first.startsWith("e") && first.endsWith(".simulateLoad")) {
//            handleExecutorSimulate(args);
        } else {
            mout << "type 'help' for help" << "\n";
        }
    }

//void handleExecutorSimulate(std::string* args) {
//std::string first = args[0];
//int threadCount = toValue<int>(first.substring(1, first.indexOf(".")));
//if (threadCount < 1 || threadCount > 16) {
//throw new RuntimeException("threadcount can't be smaller than 1 or larger than 16");
//}
//
//int taskCount = toValue<int>(args[1]);
//int durationSec = toValue<int>(args[2]);
//
//long startMs = System.time(NULL);
//
//IExecutorService executor = hazelcast.getExecutorService("e" + threadCount);
//List<Future> futures = new LinkedList<Future>();
//List<Member> members = new LinkedList<Member>(hazelcast.getCluster().getMembers());
//
//int totalThreadCount = hazelcast.getCluster().getMembers().size() * threadCount;
//
//int latchId = 0;
//for (int k = 0; k < taskCount; k++) {
//Member member = members.get(k % members.size());
//if (taskCount % totalThreadCount == 0) {
//latchId = taskCount / totalThreadCount;
//hazelcast.getCountDownLatch("latch" + latchId).trySetCount(totalThreadCount);
//
//}
//Future f = executor.submitToMember(new SimulateLoadTask(durationSec, k + 1, "latch" + latchId), member);
//futures.add(f);
//}
//
//for (Future f : futures) {
//try {
//f.get();
//} catch (std::exception&  e) {
//std::cout << e.what() << "\n";
//} catch (ExecutionException e) {
//std::cout << e.what() << "\n";
//}
//}
//
//long durationMs = System.time(NULL) - startMs;
//mout << format("Executed %s tasks in %s ms", taskCount, durationMs) << "\n";
//}

/**
* A simulated load test
*/
//static const class SimulateLoadTask implements Callable, Serializable, HazelcastInstanceAware {
//
//static const long serialVersionUID = 1;
//
//const int delay;
//const int taskId;
//const std::string latchId;
//transient HazelcastInstance hz;
//
//SimulateLoadTask(int delay, int taskId, std::string latchId) {
//    this.delay = delay;
//    this.taskId = taskId;
//    this.latchId = latchId;
//}
//
//
//void setHazelcastInstance(HazelcastInstance hazelcastInstance) {
//    this.hz = hazelcastInstance;
//}
//
//
//Object call() throws Exception {
//    try {
//        Thread.sleep(delay * ONE_THOUSAND);
//    } catch (std::exception&  e) {
//        throw new RuntimeException(e);
//    }
//
//    hz.getCountDownLatch(latchId).countDown();
//    System.out.mout << "Finished task:" << taskId << "\n";
//    return null;
//}
//}

//    void handleColon(std::string command) {
//        std::stringTokenizer st = new std::stringTokenizer(command, ";");
//        while (st.hasMoreTokens()) {
//            handleCommand(st.nextToken());
//        }
//    }
//
//    void handleAt(std::string first) {
//        if (first.length() == 1) {
//            mout << "usage: @<file-name>" << "\n";
//            return;
//        }
//        File f = new File(first.substring(1));
//        mout << "Executing script file " << f.getAbsolutePath() << "\n";
//        if (f.exists()) {
//            try {
//                BufferedReader br = new BufferedReader(new FileReader(f));
//                std::string l = br.readLine();
//                while (l != null) {
//                    handleCommand(l);
//                    l = br.readLine();
//                }
//                br.close();
//            } catch (IOException e) {
//                std::cout << e.what() << "\n";
//            }
//        } else {
//            mout << "File not found! " << f.getAbsolutePath() << "\n";
//        }
//    }

    void handleEcho(std::string command) {
        mout << command << "\n";
    }

    void handleNamespace(const std::vector<std::string> &args) {
        if (args.size() > 1) {
            _namespace = args[1];
            mout << "_namespace: " << _namespace << "\n";
        }
    }

    void handleWho() {
        std::vector<Member> members = hazelcast.getCluster().getMembers();
        std::cout << "Members [" << members.size() << "]  {" << std::endl;
        for (std::vector<Member>::iterator it = members.begin(); it != members.end(); ++it) {
            (std::cout << "\t" << (*it) << std::endl);
        }
        std::cout << "}" << std::endl;
    }
//
//    void handleAtomicNumberGet(const std::vector<std::string>& args) {
//        mout << getAtomicNumber().get() << "\n";
//    }
//
//    void handleAtomicNumberSet(const std::vector<std::string>& args) {
//        long v = 0;
//        if (args.length > 1) {
//            v = Long.valueOf(args[1]);
//        }
//        getAtomicNumber().set(v);
//        mout << getAtomicNumber().get() << "\n";
//    }
//
//    void handleAtomicNumberInc(const std::vector<std::string>& args) {
//        mout << getAtomicNumber().incrementAndGet() << "\n";
//    }
//
//    void handleAtomicNumberDec(const std::vector<std::string>& args) {
//        mout << getAtomicNumber().decrementAndGet() << "\n";
//    }

//protected:
//void handlePartitions(std::string* args) {
//Set<Partition> partitions = hazelcast.getPartitionService().getPartitions();
//Map<Member, Integer> partitionCounts = new HashMap<Member, Integer>();
//for (Partition partition : partitions) {
//Member owner = partition.getOwner();
//if (owner != null) {
//Integer count = partitionCounts.get(owner);
//int newCount = 1;
//if (count != null) {
//newCount = count + 1;
//}
//partitionCounts.put(owner, newCount);
//}
//mout << partition << "\n";
//}
//Set<Map.Entry<Member, Integer>> entries = partitionCounts.entrySet();
//for (Map.Entry<Member, Integer> entry : entries) {
//mout << entry.getKey() << ":" << entry.getValue() << "\n";
//}
//}

//    void handleInstances(const std::vector<std::string>& args) {
//        Collection<DistributedObject> distributedObjects = hazelcast.getDistributedObjects();
//        for (DistributedObject distributedObject : distributedObjects) {
//            mout << distributedObject << "\n";
//        }
//    }

// ==================== list ===================================

//    void handleListContains(const std::vector<std::string>& args) {
//        mout <<  contains(args[1]), getList()) << "\n";
//    }
//
//    void handleListRemove(const std::vector<std::string>& args) {
//        int index = -1;
//        try {
//            index = toValue<int>(args[1]);
//        } catch (NumberFormatException e) {
//            throw new RuntimeException(e);
//        }
//        if (index >= 0) {
//            mout << getList().remove(index) << "\n";
//        } else {
//            mout << getList().remove(args[1]) << "\n";
//        }
//    }
//
//    void handleListAdd(const std::vector<std::string>& args) {
//        if (args.length == 3) {
//            const int index = toValue<int>(args[1]);
//            getList().add(index, args[2]);
//            mout << "true" << "\n";
//        } else {
//            mout << getList().add(args[1]) << "\n";
//        }
//    }
//
//    void handleListSet(const std::vector<std::string>& args) {
//        const int index = toValue<int>(args[1]);
//        mout << getList().set(index, args[2]) << "\n";
//    }
//
//    void handleListAddMany(const std::vector<std::string>& args) {
//        int count = 1;
//        if (args.length > 1) {
//            count = toValue<int>(args[1]);
//        }
//        int successCount = 0;
//        long t0 = time(NULL);
//        for (int i = 0; i < count; i++) {
//            bool success = getList().add("obj" + i);
//            if (success) {
//                successCount++;
//            }
//        }
//        long t1 = time(NULL);
//        mout << "Added " << successCount << " objects." << "\n";
//        println("size = " + list.size() + ", " + successCount * ONE_THOUSAND / (t1 - t0)
//                + " evt/s");
//    }

// ==================== map ===================================

    void handleMapPut(const std::vector<std::string> &args) {
        moutPtr << getMap().put(args[1], args[2]) << "\n";
    }

//    void handleMapPutAsync(const std::vector<std::string>& args) {
//        try {
//            mout << getMap().putAsync(args[1], args[2]).get() << "\n";
//        } catch (std::exception&  e) {
//            std::cout << e.what() << "\n";
//        } catch (ExecutionException e) {
//            std::cout << e.what() << "\n";
//        }
//    }

    void handleMapPutIfAbsent(const std::vector<std::string> &args) {
        moutPtr << getMap().putIfAbsent(args[1], args[2]) << "\n";
    }

    void handleMapReplace(const std::vector<std::string> &args) {
        moutPtr << getMap().replace(args[1], args[2]) << "\n";
    }

    void handleMapGet(const std::vector<std::string> &args) {
        moutPtr << getMap().get(args[1]) << "\n";
    }

//    void handleMapGetAsync(const std::vector<std::string>& args) {
//        try {
//            mout << getMap().getAsync(args[1]).get() << "\n";
//        } catch (std::exception&  e) {
//            std::cout << e.what() << "\n";
//        } catch (ExecutionException e) {
//            std::cout << e.what() << "\n";
//        }
//    }

    void handleMapGetMapEntry(const std::vector<std::string> &args) {
        EntryView<std::string, std::string> view = getMap().getEntryView(args[1]);
        mout << view.key << " : " << view.value << "\n";
    }

    void handleMapRemove(const std::vector<std::string> &args) {
        moutPtr << getMap().remove(args[1]) << "\n";
    }

    void handleMapEvict(const std::vector<std::string> &args) {
        mout << getMap().evict(args[1]) << "\n";
    }

    void handleMapPutMany(const std::vector<std::string> &args) {
        int count = 1;
        if (args.size() > 1) {
            count = toValue<int>(args[1]);
        }
        int b = ONE_HUNDRED;
        std::string value(b, '1');
        if (args.size() > 2) {
            b = toValue<int>(args[2]);
            value = std::string(b, '1');
        }
        int start = getMap().size();
        if (args.size() > 3) {
            start = toValue<int>(args[3]);
        }
        std::map<std::string, std::string > theMap;
        for (int i = 0; i < count; i++) {
            theMap[std::string("key") + toString((start + i))] = value;
        }
        time_t t0 = time(NULL);
        getMap().putAll(theMap);
        time_t t1 = time(NULL);
        if (t1 - t0 > 1) {
            mout << "size = " << getMap().size() << ", " << (count * ONE_THOUSAND / difftime(t1, t0))
                    << " evt/s, " << ((count * ONE_THOUSAND / (t1 - t0)) * (b * 8) / ONE_KB) << " Kbit/s, "
                    << count * b / ONE_KB << " KB added" << "\n";
        }
    }

    void handleMapGetMany(const std::vector<std::string> &args) {
        int count = 1;
        if (args.size() > 1) {
            count = toValue<int>(args[1]);
        }
        for (int i = 0; i < count; i++) {
            mout << getMap().get(std::string("key") + toString(i)) << "\n";
        }
    }

    void handleMapRemoveMany(const std::vector<std::string> &args) {
        int count = 1;
        if (args.size() > 1) {
            count = toValue<int>(args[1]);
        }
        int start = 0;
        if (args.size() > 2) {
            start = toValue<int>(args[2]);
        }
        time_t t0 = time(NULL);
        for (int i = 0; i < count; i++) {
            getMap().remove(std::string("key") + toString(start + i));
        }
        time_t t1 = time(NULL);
        mout << "size = " << getMap().size() << ", " << count * ONE_THOUSAND / difftime(t1, t0) << " evt/s" << "\n";
    }

    void handleMapLock(const std::vector<std::string> &args) {
        getMap().lock(args[1]);
        mout << "true" << "\n";
    }

    void handleMapTryLock(const std::vector<std::string> &args) {
        std::string key = args[1];
        long time = (args.size() > 2) ? toValue<long>(args[2]) : 0;
        bool locked;
        if (time == 0) {
            locked = getMap().tryLock(key);
        } else {
            try {
                locked = getMap().tryLock(key, time);
            } catch (std::exception &) {
                locked = false;
            }
        }
        mout << locked << "\n";
    }

    void handleMapUnlock(const std::vector<std::string> &args) {
        getMap().unlock(args[1]);
        mout << "true" << "\n";
    }

    void handleMapKeys() {
        std::vector<std::string> keys = getMap().keySet();
        std::vector<std::string>::iterator it;
        int count = 0;
        for (it = keys.begin(); it != keys.end(); ++it) {
            count++;
            mout << *it << "\n";
        }
        mout << "Total " << count << "\n";
    }

    void handleMapEntries() {
        std::vector<std::pair<std::string, std::string> > entries = getMap().entrySet();
        std::vector<std::pair<std::string, std::string> >::iterator it;
        int count = 0;
        for (it = entries.begin(); it != entries.end(); ++it) {
            count++;
            mout << it->first << " : " << it->second << "\n";
        }
        mout << "Total " << count << "\n";
    }

    void handleMapValues() {
        std::vector<std::string> v = getMap().values();
        std::vector<std::string>::iterator it;
        int count = 0;
        for (it = v.begin(); it != v.end(); ++it) {
            count++;
            mout << *it << "\n";
        }
        mout << "Total " << count << "\n";
    }

// ==================== multimap ===================================

//    void handleMultiMapPut(const std::vector<std::string>& args) {
//        mout << getMultiMap().put(args[1], args[2]) << "\n";
//    }
//
//    void handleMultiMapGet(const std::vector<std::string>& args) {
//        mout << getMultiMap().get(args[1]) << "\n";
//    }
//
//    void handleMultiMapRemove(const std::vector<std::string>& args) {
//        mout << getMultiMap().remove(args[1]) << "\n";
//    }
//
//    void handleMultiMapKeys() {
//        Set set = getMultiMap().keySet();
//        Iterator it = set.iterator();
//        int count = 0;
//        while (it.hasNext()) {
//            count++;
//            mout << it.next() << "\n";
//        }
//        mout << "Total " << count << "\n";
//    }
//
//    void handleMultiMapEntries() {
//        Set set = getMultiMap().entrySet();
//        Iterator it = set.iterator();
//        int count = 0;
//        while (it.hasNext()) {
//            count++;
//            Map.Entry
//            entry = (Entry) it.next();
//            mout << entry.getKey() << " : " << entry.getValue() << "\n";
//        }
//        mout << "Total " << count << "\n";
//    }
//
//    void handleMultiMapValues() {
//        Collection set = getMultiMap().values();
//        Iterator it = set.iterator();
//        int count = 0;
//        while (it.hasNext()) {
//            count++;
//            mout << it.next() << "\n";
//        }
//        mout << "Total " << count << "\n";
//    }
//
//    void handleMultiMapLock(const std::vector<std::string>& args) {
//        getMultiMap().lock(args[1]);
//        mout << "true" << "\n";
//    }
//
//    void handleMultiMapTryLock(const std::vector<std::string>& args) {
//        std::string key = args[1];
//        long time = (args.size() > 2) ? Long.valueOf(args[2]) : 0;
//        bool locked;
//        if (time == 0) {
//            locked = getMultiMap().tryLock(key);
//        } else {
//            try {
//                locked = getMultiMap().tryLock(key, time);
//            } catch (std::exception&  e) {
//                locked = false;
//            }
//        }
//        mout << locked << "\n";
//    }
//
//    void handleMultiMapUnlock(const std::vector<std::string>& args) {
//        getMultiMap().unlock(args[1]);
//        mout << "true" << "\n";
//    }

// =======================================================

//    void handStats(const std::vector<std::string>& args) {
//
//        if (startsWith(command, "m.")) {
//            mout << getMap().getLocalMapStats() << "\n";
//        } else if (startsWith(command, "mm.")) {
//            mout << getMultiMap().getLocalMultiMapStats() << "\n";
//        } else if (startsWith(command, "q.")) {
//            mout << getQueue().getLocalQueueStats() << "\n";
//        }
//    }
//
//
//    void handleLock(const std::vector<std::string>& args) {
//        std::string lockStr = args[0];
//        std::string key = args[1];
//        Lock lock = hazelcast.getLock(key);
//        if( equals("lock", lockStr)) {
//            lock.lock();
//            mout << "true" << "\n";
//        } else if( equals("unlock", lockStr)) {
//            lock.unlock();
//            mout << "true" << "\n";
//        } else if( equals("trylock", lockStr)) {
//            std::string timeout = args.size() > 2 ? args[2] : null;
//            if (timeout == null) {
//                mout << lock.tryLock() << "\n";
//            } else {
//                long time = Long.valueOf(timeout);
//                try {
//                    mout << lock.tryLock(time) << "\n";
//                } catch (std::exception&  e) {
//                    std::cout << e.what() << "\n";
//                }
//            }
//        }
//    }
//
//    void handleAddListener(const std::vector<std::string>& args) {
//        std::string first = args[0];
//        if (first.startsWith("s.")) {
//            getSet().addItemListener(this, true);
//        } else if (first.startsWith("m.")) {
//            if (args.size() > 1) {
//                getMap().addEntryListener(this, args[1], true);
//            } else {
//                getMap().addEntryListener(this, true);
//            }
//        } else if (first.startsWith("mm.")) {
//            if (args.size() > 1) {
//                getMultiMap().addEntryListener(this, args[1], true);
//            } else {
//                getMultiMap().addEntryListener(this, true);
//            }
//        } else if (first.startsWith("q.")) {
//            getQueue().addItemListener(this, true);
//        } else if (first.startsWith("t.")) {
//            getTopic().addMessageListener(this);
//        } else if (first.startsWith("l.")) {
//            getList().addItemListener(this, true);
//        }
//    }

//    void handleRemoveListener(const std::vector<std::string>& args) {
////        std::string first = args[0];
////        if (first.startsWith("s.")) {
////            getSet().removeItemListener(this);
////        } else if (first.startsWith("m.")) {
////            if (args.size() > 1) {
////                // todo revise here
////                getMap().removeEntryListener(args[1]);
////            } else {
////                getMap().removeEntryListener(args[0]);
////            }
////        } else if (first.startsWith("q.")) {
////            getQueue().removeItemListener(this);
////        } else if (first.startsWith("t.")) {
////            getTopic().removeMessageListener(this);
////        } else if (first.startsWith("l.")) {
////            getList().removeItemListener(this);
////        }
//    }

    //----------- SET BEGIN --//
//    void handleSetAdd(const std::vector<std::string>& args) {
//        mout << getSet().add(args[1]) << "\n";
//    }
//
//    void handleSetRemove(const std::vector<std::string>& args) {
//        mout << getSet().remove(args[1]) << "\n";
//    }
//
//    void handleSetAddMany(const std::vector<std::string>& args) {
//        int count = 1;
//        if (args.size() > 1) {
//            count = toValue<int>(args[1]);
//        }
//        int successCount = 0;
//        long t0 = time(NULL);
//        for (int i = 0; i < count; i++) {
//            bool success = getSet().add("obj" + i);
//            if (success) {
//                successCount++;
//            }
//        }
//        long t1 = time(NULL);
//        mout << "Added " << successCount << " objects." << "\n";
//        println("size = " + getSet().size() + ", " + successCount * ONE_THOUSAND / (t1 - t0)
//                + " evt/s");
//    }
//
//    void handleSetRemoveMany(const std::vector<std::string>& args) {
//        int count = 1;
//        if (args.size() > 1) {
//            count = toValue<int>(args[1]);
//        }
//        int successCount = 0;
//        long t0 = time(NULL);
//        for (int i = 0; i < count; i++) {
//            bool success = getSet().remove("obj" + i);
//            if (success) {
//                successCount++;
//            }
//        }
//        long t1 = time(NULL);
//        mout << "Removed " << successCount << " objects." << "\n";
//        println("size = " + getSet().size() + ", " + successCount * ONE_THOUSAND / (t1 - t0)
//                + " evt/s");
//    }
    //----------- SET END --//

    void handleIterator(const std::vector<std::string> &args) {
        std::vector<std::string> v;
        std::string command = args[0];
        bool remove = false;
        if (args.size() > 1) {
            std::string removeStr = args[1];
            remove = equals(removeStr, "remove");
        }
        if (startsWith(command, "s.")) {
//            it = getSet();
        } else if (startsWith(command, "m.")) {
            v = getMap().keySet();
            if (remove) {
                getMap().clear();
            }
        } else if (startsWith(command, "mm.")) {
//            it = getMultiMap().keySet().iterator();
        } else if (startsWith(command, "q.")) {
            v = getQueue().toArray();
            if (remove) {
                getQueue().clear();
            }
        } else if (startsWith(command, "l.")) {
//            it = getList().iterator();
        }
        std::vector<std::string>::iterator it;

        int count = 1;
        for (it = v.begin(); it != v.end(); ++it) {
            mout << count++ << " " << *it << "\n";
        }
        mout << "\n";
    }

    void handleContains(const std::vector<std::string> &args) {
        std::string iteratorStr = args[0];
        bool key = false;
//        bool value = false;
        if (endsWith(iteratorStr, "key")) {
            key = true;
        } else if (endsWith(iteratorStr, "value")) {
//            value = true;
        }
        std::string data = args[1];
        bool result = false;
        if (startsWith(iteratorStr, "s.")) {
//            result = getSet().contains(data);
        } else if (startsWith(iteratorStr, "m.")) {
            result = (key) ? getMap().containsKey(data) : getMap().containsValue(data);
        } else if (startsWith(iteratorStr, "mmm.")) {
//            result = (key) ? getMultiMap().containsKey(data) : getMultiMap().containsValue(data);
        } else if (startsWith(iteratorStr, "q.")) {
            result = getQueue().contains(data);
        } else if (startsWith(iteratorStr, "l.")) {
//            result = getList().contains(data);
        }
        mout << "Contains : " << result << "\n";
    }

    void handleSize(const std::vector<std::string> &args) {
        int size = 0;

        if (startsWith(args[0], "s.")) {
//            size = getSet().size();
        } else if (startsWith(args[0], "m.")) {
            size = getMap().size();
        } else if (startsWith(args[0], "mm.")) {
//            size = getMultiMap().size();
        } else if (startsWith(args[0], "q.")) {
            size = getQueue().size();
        } else if (startsWith(args[0], "l.")) {
//            size = getList().size();
        }
        mout << "Size = " << size << "\n";
    }

    void handleClear(const std::vector<std::string> &args) {
        if (startsWith(args[0], "s.")) {
//            getSet().clear();
        } else if (startsWith(args[0], "m.")) {
            getMap().clear();
        } else if (startsWith(args[0], "mm.")) {
//            getMultiMap().clear();
        } else if (startsWith(args[0], "q.")) {
            getQueue().clear();
        } else if (startsWith(args[0], "l.")) {
//            getList().clear();
        }
        mout << "Cleared all." << "\n";
    }

    void handleDestroy(const std::vector<std::string> &args) {

        if (startsWith(args[0], "s.")) {
//            getSet().destroy();
        } else if (startsWith(args[0], "m.")) {
            getMap().destroy();
        } else if (startsWith(args[0], "mm.")) {
//            getMultiMap().destroy();
        } else if (startsWith(args[0], "q.")) {
            getQueue().destroy();
        } else if (startsWith(args[0], "l.")) {
//            getList().destroy();
        } else if (startsWith(args[0], "t.")) {
//            getTopic().destroy();
        }
        mout << "Destroyed!" << "\n";
    }

    void handleQOffer(const std::vector<std::string> &args) {
        long timeout = 0;
        if (args.size() > 2) {
            timeout = toValue<long>(args[2]);
        }
        try {
            bool offered = getQueue().offer(args[1], timeout);
            mout << offered << "\n";
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }

    void handleQTake(const std::vector<std::string> &args) {
        try {
            moutPtr << getQueue().take() << "\n";
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }

    void handleQPoll(const std::vector<std::string> &args) {
        long timeout = 0;
        if (args.size() > 1) {
            timeout = toValue<long>(args[1]);
        }
        try {
            moutPtr << getQueue().poll(timeout) << "\n";
        } catch (std::exception &e) {
            std::cout << e.what() << "\n";
        }
    }

    void handleQOfferMany(const std::vector<std::string> &args) {
        int count = 1;
        if (args.size() > 1) {
            count = toValue<int>(args[1]);
        }


        time_t t0 = time(NULL);
        for (int i = 0; i < count; i++) {
            getQueue().offer("obj");
        }
        time_t t1 = time(NULL);
        mout << "size = " << getQueue().size() << ", " << (count * ONE_THOUSAND / difftime(t1, t0)) << " evt/s" << "\n";
    }

    void handleQPollMany(const std::vector<std::string> &args) {
        int count = 1;
        if (args.size() > 1) {
            count = toValue<int>(args[1]);
        }
        int c = 1;
        for (int i = 0; i < count; i++) {
            boost::shared_ptr<std::string> obj = getQueue().poll();
            if (obj == NULL) {
                break;
            }
            mout << c++ << " " << *obj << "\n";
        }
    }

    void handleQPeek(const std::vector<std::string> &args) {
        mout << getQueue().peek() << "\n";
    }

    void handleQCapacity(const std::vector<std::string> &args) {
        mout << getQueue().remainingCapacity() << "\n";
    }

//    void handleTopicPublish(const std::vector<std::string>& args) {
//        getTopic().publish(args[1]);
//    }

private:
//void execute(std::string* args) {
//// execute <echo-string>
//doExecute(false, false, args);
//}
//
//void executeOnKey(std::string* args) {
//// executeOnKey <echo-string> <key>
//doExecute(true, false, args);
//}
//
//void executeOnMember(std::string* args) {
//// executeOnMember <echo-string> <memberIndex>
//doExecute(false, true, args);
//}
//
//void doExecute(bool onKey, bool onMember, std::string* args) {
//    // executeOnKey <echo-string> <key>
//    try {
//        IExecutorService executorService = hazelcast.getExecutorService("default");
//        Echo callable = new Echo(args[1]);
//        Future<std::string> future;
//        if (onKey) {
//            std::string key = args[2];
//            future = executorService.submitToKeyOwner(callable, key);
//        } else if (onMember) {
//            int memberIndex = toValue<int>(args[2]);
//            List<Member> members = new LinkedList(hazelcast.getCluster().getMembers());
//            if (memberIndex >= members.size()) {
//                throw new IndexOutOfBoundsException("Member index: " + memberIndex + " must be smaller than " + members
//                        .size());
//            }
//            Member member = members.get(memberIndex);
//            future = executorService.submitToMember(callable, member);
//        } else {
//            future = executorService.submit(callable);
//        }
//        mout << "Result: " << future.get() << "\n";
//    } catch (std::exception&  e) {
//        std::cout << e.what() << "\n";
//    } catch (ExecutionException e) {
//        std::cout << e.what() << "\n";
//    }
//}
//
//void executeOnMembers(std::string* args) {
//// executeOnMembers <echo-string>
//try {
//IExecutorService executorService = hazelcast.getExecutorService("default");
//Echo task = new Echo(args[1]);
//Map<Member, Future<std::string>> results = executorService.submitToAllMembers(task);
//
//for (Future f : results.values()) {
//mout << f.get() << "\n";
//}
//} catch (std::exception&  e) {
//std::cout << e.what() << "\n";
//} catch (ExecutionException e) {
//std::cout << e.what() << "\n";
//}
//}
//
//
//void entryAdded(EntryEvent event) {
//    mout << event << "\n";
//}
//
//
//void entryRemoved(EntryEvent event) {
//    mout << event << "\n";
//}
//
//
//void entryUpdated(EntryEvent event) {
//    mout << event << "\n";
//}
//
//
//void entryEvicted(EntryEvent event) {
//    mout << event << "\n";
//}
//
//
//void itemAdded(ItemEvent itemEvent) {
//    mout << "Item added = " << itemEvent.getItem() << "\n";
//}
//
//
//void itemRemoved(ItemEvent itemEvent) {
//    mout << "Item removed = " << itemEvent.getItem() << "\n";
//}
//
//
//void onMessage(Message<std::sttng> msg) {
//    mout << "Topic received = " << msg.getMessageObject() << "\n";
//}

///**
// * Echoes to screen
// */
//static class Echo extends HazelcastInstanceAwareObject implements Callable<std::string>, DataSerializable {
//
//std::string input;
//
//Echo() {
//}
//
//Echo(std::string input) {
//this.input = input;
//}
//
//
//std::string call() {
//    getHazelcastInstance().getCountDownLatch("latch").countDown();
//    return getHazelcastInstance().getCluster().getLocalMember().tostd::string() + ":" + input;
//}
//
//
//void writeData(ObjectDataOutput out) throws IOException {
//    out.writeUTF(input);
//}
//
//
//void readData(ObjectDataInput in) throws IOException {
//    input = in.readUTF();
//}
//}

///**
// * A Hazelcast instance aware object
// */
//static class HazelcastInstanceAwareObject implements HazelcastInstanceAware {
//
//HazelcastInstance hazelcastInstance;
//
//HazelcastInstance getHazelcastInstance() {
//    return hazelcastInstance;
//}
//
//
//void setHazelcastInstance(HazelcastInstance hazelcastInstance) {
//    this.hazelcastInstance = hazelcastInstance;
//}
//}

/**
* Handled the help command
* @param command
*/
    void handleHelp(const std::vector<std::string> &args) {
        bool silentBefore = silent;
        silent = false;
        mout << "Commands:" << "\n";

        printGeneralCommands();
        printQueueCommands();
//    printSetCommands();
//    printLockCommands();
        printMapCommands();
//    printMulitiMapCommands();
//    printListCommands();
//    printAtomicLongCommands();
//    printExecutorServiceCommands();

        silent = silentBefore;
    }

    void printGeneralCommands() {
        mout << "-- General commands" << "\n";
        mout << "echo true|false                      //turns on/off echo of commands (default false)" << "\n";
        mout << "silent true|false                    //turns on/off silent of command output (default false)" << "\n";
//        mout << "#<number> <command>                  //repeats <number> time <command>, replace $i in <command> with current iteration (0..<number-1>)" << "\n";
//        mout << "&<number> <command>                  //forks <number> threads to execute <command>, replace $t in <command> with current thread number (0..<number-1>" << "\n";
//        mout << "     When using #x or &x, is is advised to use silent true as well." << "\n";
//        mout << "     When using &x with m.putmany and m.removemany, each thread will get a different share of keys unless a start key index is specified" << "\n";
        mout << "who                                  //displays info about the cluster" << "\n";
        mout << "ns <string>                          //switch the namespace for using the distributed queue/map/set/list <string> (defaults to \"default\"" << "\n";
//        mout << "@<file>                              //executes the given <file> script. Use '//' for comments in the script" << "\n";
        mout << "\n";
    }

    void printQueueCommands() {
        mout << "-- Queue commands" << "\n";
        mout << "q.offer <string>                     //adds a string object to the queue" << "\n";
        mout << "q.poll                               //takes an object from the queue" << "\n";
        mout << "q.offermany <number> [<size>]        //adds indicated number of string objects to the queue ('obj<i>' or byte[<size>]) " << "\n";
        mout << "q.pollmany <number>                  //takes indicated number of objects from the queue" << "\n";
        mout << "q.iterator [remove]                  //iterates the queue, remove if specified" << "\n";
        mout << "q.size                               //size of the queue" << "\n";
        mout << "q.clear                              //clears the queue" << "\n";
        mout << "\n";
    }

    void printSetCommands() {
        mout << "-- Set commands" << "\n";
        mout << "s.add <string>                       //adds a string object to the set" << "\n";
        mout << "s.remove <string>                    //removes the string object from the set" << "\n";
        mout << "s.addmany <number>                   //adds indicated number of string objects to the set ('obj<i>')" << "\n";
        mout << "s.removemany <number>                //takes indicated number of objects from the set" << "\n";
        mout << "s.iterator [remove]                  //iterates the set, removes if specified" << "\n";
        mout << "s.size                               //size of the set" << "\n";
        mout << "s.clear                              //clears the set" << "\n";
        mout << "\n";
    }

    void printLockCommands() {
        mout << "-- Lock commands" << "\n";
        mout << "lock <key>                           //same as Hazelcast.getLock(key).lock()" << "\n";
        mout << "tryLock <key>                        //same as Hazelcast.getLock(key).tryLock()" << "\n";
        mout << "tryLock <key> <time>                 //same as tryLock <key> with timeout in seconds" << "\n";
        mout << "unlock <key>                         //same as Hazelcast.getLock(key).unlock()" << "\n";
        mout << "\n";
    }

    void printMapCommands() {
        mout << "-- Map commands" << "\n";
        mout << "m.put <key> <value>                  //puts an entry to the map" << "\n";
        mout << "m.remove <key>                       //removes the entry of given key from the map" << "\n";
        mout << "m.get <key>                          //returns the value of given key from the map" << "\n";
        mout << "m.putmany <number> [<size>] [<index>]//puts indicated number of entries to the map ('key<i>':byte[<size>], <index>+(0..<number>)" << "\n";
        mout << "m.removemany <number> [<index>]      //removes indicated number of entries from the map ('key<i>', <index>+(0..<number>)" << "\n";
        mout << "     When using &x with m.putmany and m.removemany, each thread will get a different share of keys unless a start key <index> is specified" << "\n";
        mout << "m.keys                               //iterates the keys of the map" << "\n";
        mout << "m.values                             //iterates the values of the map" << "\n";
        mout << "m.entries                            //iterates the entries of the map" << "\n";
        mout << "m.iterator [remove]                  //iterates the keys of the map, remove if specified" << "\n";
        mout << "m.size                               //size of the map" << "\n";
        mout << "m.clear                              //clears the map" << "\n";
        mout << "m.destroy                            //destroys the map" << "\n";
        mout << "m.lock <key>                         //locks the key" << "\n";
        mout << "m.tryLock <key>                      //tries to lock the key and returns immediately" << "\n";
        mout << "m.tryLock <key> <time>               //tries to lock the key within given seconds" << "\n";
        mout << "m.unlock <key>                       //unlocks the key" << "\n";
        mout << "\n";
    }

    void printMulitiMapCommands() {
        mout << "-- MultiMap commands" << "\n";
        mout << "mm.put <key> <value>                  //puts an entry to the multimap" << "\n";
        mout << "mm.get <key>                          //returns the value of given key from the multimap" << "\n";
        mout << "mm.remove <key>                       //removes the entry of given key from the multimap" << "\n";
        mout << "mm.size                               //size of the multimap" << "\n";
        mout << "mm.clear                              //clears the multimap" << "\n";
        mout << "mm.destroy                            //destroys the multimap" << "\n";
        mout << "mm.iterator [remove]                  //iterates the keys of the multimap, remove if specified" << "\n";
        mout << "mm.keys                               //iterates the keys of the multimap" << "\n";
        mout << "mm.values                             //iterates the values of the multimap" << "\n";
        mout << "mm.entries                            //iterates the entries of the multimap" << "\n";
        mout << "mm.lock <key>                         //locks the key" << "\n";
        mout << "mm.tryLock <key>                      //tries to lock the key and returns immediately" << "\n";
        mout << "mm.tryLock <key> <time>               //tries to lock the key within given seconds" << "\n";
        mout << "mm.unlock <key>                       //unlocks the key" << "\n";
        mout << "mm.stats                              //shows the local stats of the multimap" << "\n";
        mout << "\n";
    }

    void printExecutorServiceCommands() {
        mout << "-- Executor Service commands:" << "\n";
        mout << "execute <echo-input>                            //executes an echo task on random member" << "\n";
        mout << "executeOnKey <echo-input> <key>                  //executes an echo task on the member that owns the given key" << "\n";
        mout << "executeOnMember <echo-input> <memberIndex>         //executes an echo task on the member with given index" << "\n";
        mout << "executeOnMembers <echo-input>                      //executes an echo task on all of the members" << "\n";
        mout << "e<threadcount>.simulateLoad <task-count> <delaySeconds>        //simulates load on executor with given number of thread (e1..e16)" << "\n";

        mout << "\n";
    }

    void printAtomicLongCommands() {
        mout << "-- IAtomicLong commands:" << "\n";
        mout << "a.get" << "\n";
        mout << "a.set <long>" << "\n";
        mout << "a.inc" << "\n";
        mout << "a.dec" << "\n";
    }

    void printListCommands() {
        mout << "-- List commands:" << "\n";
        mout << "l.add <string>" << "\n";
        mout << "l.add <index> <string>" << "\n";
        mout << "l.contains <string>" << "\n";
        mout << "l.remove <string>" << "\n";
        mout << "l.remove <index>" << "\n";
        mout << "l.set <index> <string>" << "\n";
        mout << "l.iterator [remove]" << "\n";
        mout << "l.size" << "\n";
        mout << "l.clear" << "\n";
    }
};

int main() {

    ClientConfig config;
    config.addAddress(Address("127.0.0.1", 5701));
    HazelcastClient client(config);

    CommandLineTool testApp(client);
    testApp.start();
    return 0;
}

