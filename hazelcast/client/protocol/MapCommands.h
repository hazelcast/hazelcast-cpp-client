#ifndef HAZELCAST_MAP_COMMANDS
#define HAZELCAST_MAP_COMMANDS


#include "Command.h"
#include "../serialization/DataInput.h"
#include "../serialization/DataOutput.h"
#include "HazelcastException.h"
#include <boost/thread.hpp>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <cstdlib>
#include <cstdio>

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace MapCommands {

                static std::string NEWLINE = "\r\n";
                static std::string SPACE = " ";

                class ContainsKeyCommand : public Command {
                public:

                    ContainsKeyCommand(std::string instanceName, hazelcast::client::serialization::Data key) : instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MCONTAINSKEY";
                        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;
                        dataOutput.write(command.c_str(), command.length());

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        dataOutput.write(integerBuffer, integerBufferSize);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of containsKey return");

                        containsKey = line.compare("OK true") ? false : true;
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };

                    bool get() {
                        return containsKey;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    bool containsKey;
                };

                class ContainsValueCommand : public Command {
                public:

                    ContainsValueCommand(std::string instanceName, hazelcast::client::serialization::Data value) : instanceName(instanceName)
                    , value(value) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MCONTAINSVALUE";
                        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;
                        dataOutput.write(command.c_str(), command.length());

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%d", value.totalSize());
                        dataOutput.write(integerBuffer, integerBufferSize);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());

                        value.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of containsValue return");

                        containsValue = line.compare("OK true") ? false : true;
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };

                    bool get() {
                        return containsValue;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data value;
                    bool containsValue;
                };

                class PutCommand : public Command {
                public:

                    PutCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key,
                            hazelcast::client::serialization::Data value,
                            long ttl)
                    : instanceName(instanceName)
                    , key(key)
                    , value(value)
                    , ttl(ttl) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MPUT";
                        command += SPACE + instanceName + SPACE;
                        char integerBuffer[20];
                        int integerBufferSize;

                        integerBufferSize = sprintf(integerBuffer, "%li", ttl);
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        command += "#2" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        integerBufferSize = sprintf(integerBuffer, "%d", value.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);
                        value.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (!line.compare("OK"))
                            numResults = 0;
                        else if (!line.compare("OK #1"))
                            numResults = 1;
                        else
                            throw hazelcast::client::HazelcastException("unexpected header of put return");
                    };

                    void readSizeLine(std::string line) {
                        returnSize = atoi(line.c_str());
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return numResults;
                    };

                    int resultSize(int i) {
                        return returnSize;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    hazelcast::client::serialization::Data value;
                    long ttl;
                    int returnSize;
                    int numResults;
                };

                class GetCommand : public Command {
                public:

                    GetCommand(std::string instanceName, hazelcast::client::serialization::Data key) : instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MGET";
                        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);
                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (!line.compare("OK"))
                            numResults = 0;
                        else if (!line.compare("OK #1"))
                            numResults = 1;
                        else
                            throw hazelcast::client::HazelcastException("unexpected header of get return");
                    };

                    void readSizeLine(std::string line) {
                        int returnSize = atoi(line.c_str());
                        resultSizes.push_back(returnSize);
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                        oldValue.readData(dataInput);
                    };

                    int nResults() {
                        return numResults;
                    };

                    int resultSize(int i) {
                        return resultSizes[i];
                    };

                    hazelcast::client::serialization::Data get() {
                        return oldValue;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    std::vector<int> resultSizes;
                    int numResults;
                    hazelcast::client::serialization::Data oldValue;
                };

                class RemoveCommand : public Command {
                public:

                    RemoveCommand(std::string instanceName, hazelcast::client::serialization::Data key) : instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MREMOVE";
                        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);
                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (line.compare("OK #1"))
                            throw hazelcast::client::HazelcastException("unexpected header of remove return");
                    };

                    void readSizeLine(std::string line) {
                        returnSize = atoi(line.c_str());
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 1;
                    };

                    int resultSize(int i) {
                        return returnSize;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    hazelcast::client::serialization::Data oldValue;
                    int returnSize;
                };

                class FlushCommand : public Command {
                public:

                    FlushCommand(std::string instanceName) : instanceName(instanceName) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MFLUSH";
                        command += SPACE + instanceName + NEWLINE;
                        dataOutput.write(command.c_str(), command.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (line.compare("OK "))
                            throw hazelcast::client::HazelcastException("unexpected header of flush return");

                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };
                private:
                    std::string instanceName;
                };

                class GetAllCommand : public Command {
                public:
                    typedef std::vector<hazelcast::client::serialization::Data> DataSet;

                    GetAllCommand(std::string instanceName, std::vector<hazelcast::client::serialization::Data> keySet) : instanceName(instanceName)
                    , keySet(keySet) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MGETALL";
                        command += SPACE + instanceName + SPACE + "#";

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%d", (int) keySet.size());
                        command.append(integerBuffer, integerBufferSize);
                        command += NEWLINE;

                        for (DataSet::const_iterator it = keySet.begin(); it != keySet.end(); it++) {
                            char integerBuffer[20];
                            int integerBufferSize = sprintf(integerBuffer, "%d", it->totalSize());
                            command.append(integerBuffer, integerBufferSize);

                            command += SPACE;
                        }
                        command.erase(command.end() - 1);
                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());
                        for (DataSet::const_iterator it = keySet.begin(); it != keySet.end(); it++) {
                            it->writeData(dataOutput);
                        }
                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of('#');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK "))
                            throw hazelcast::client::HazelcastException("unexpected header of getAll return");
                        std::string sizeStr = line.substr(pos + 1, line.length() - pos);
                        nReturnedResults = std::atoi(sizeStr.c_str());
                        nReturnedResults /= 2;
                    };

                    void readSizeLine(std::string line) {
                        int beg = 0, end = 0, middle = 0;
                        do {
                            middle = line.find_first_of(" ", beg);
                            end = line.find_first_of(" ", middle + 1);

                            std::string keySize = line.substr(beg, middle - beg);
                            int a = atoi(keySize.c_str());
                            keySizes.push_back(a);

                            std::string valueSize = line.substr(middle, end - middle);
                            int b = atoi(valueSize.c_str());
                            valueSizes.push_back(b);

                            beg = end;
                            beg++;

                        } while (end != std::string::npos);

                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                        using namespace hazelcast::client::serialization;
                        Data key;
                        key.readData(dataInput);
                        keys.push_back(key);

                        Data value;
                        value.readData(dataInput);
                        values.push_back(value);

                    };

                    int nResults() {
                        return nReturnedResults;
                    };

                    int resultSize(int i) {
                        return keySizes[i] + valueSizes[i];
                    };

                    DataSet getKeys() {
                        return keys;
                    };

                    DataSet getValues() {
                        return values;
                    };
                private:
                    std::string instanceName;
                    DataSet keySet;
                    int nReturnedResults;
                    std::vector<int> keySizes;
                    std::vector<int> valueSizes;
                    DataSet keys;
                    DataSet values;
                };

                class TryRemoveCommand : public Command {
                public:

                    TryRemoveCommand(std::string instanceName, hazelcast::client::serialization::Data key, long timeoutInMillis)
                    : instanceName(instanceName)
                    , key(key)
                    , timeoutInMillis(timeoutInMillis) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MTRYREMOVE";
                        command += SPACE + instanceName + SPACE;

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%li", timeoutInMillis);
                        command.append(integerBuffer, integerBufferSize);

                        command += SPACE + "#1" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (!line.compare("OK #1")) {
                            success = true;
                            numResults = 1;
                        } else if (!line.compare("OK")) {
                            success = true;
                            numResults = 0;
                        } else if (!line.compare("OK timeout"))
                            success = false;
                        else
                            throw hazelcast::client::HazelcastException("unexpected header of tryRemove return");

                    };

                    void readSizeLine(std::string line) {
                        returnSize = atoi(line.c_str());
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return numResults;
                    };

                    int resultSize(int i) {
                        return returnSize;
                    };

                    bool get() {
                        return success;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    long timeoutInMillis;
                    bool success;
                    int returnSize;
                    int numResults;
                };

                class TryPutCommand : public Command {
                public:

                    TryPutCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key,
                            hazelcast::client::serialization::Data value,
                            long timeoutInMillis)
                    : instanceName(instanceName)
                    , key(key)
                    , value(value)
                    , timeoutInMillis(timeoutInMillis) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MTRYPUT";
                        command += SPACE + instanceName + SPACE + "#2" + NEWLINE;
                        dataOutput.write(command.c_str(), command.length());

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        dataOutput.write(integerBuffer, integerBufferSize);

                        dataOutput.write(SPACE.c_str(), SPACE.length());

                        integerBufferSize = sprintf(integerBuffer, "%d", value.totalSize());
                        dataOutput.write(integerBuffer, integerBufferSize);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());

                        key.writeData(dataOutput);
                        value.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of tryPut return");

                        success = line.compare("OK true") ? false : true;
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };

                    bool get() {
                        return success;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    hazelcast::client::serialization::Data value;
                    bool success;
                    long timeoutInMillis;
                };

                class PutTransientCommand : public Command {
                public:

                    PutTransientCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key,
                            hazelcast::client::serialization::Data value,
                            long ttl)
                    : instanceName(instanceName)
                    , key(key)
                    , value(value)
                    , ttl(ttl) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MPUTTRANSIENT";
                        command += SPACE + instanceName + SPACE;
                        char integerBuffer[20];
                        int integerBufferSize;

                        integerBufferSize = sprintf(integerBuffer, "%li", ttl);
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        command += "#2" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        integerBufferSize = sprintf(integerBuffer, "%d", value.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);
                        value.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (line.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of putTransient return");
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    hazelcast::client::serialization::Data value;
                    long ttl;
                };

                class PutIfAbsentCommand : public Command {
                public:

                    PutIfAbsentCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key,
                            hazelcast::client::serialization::Data value,
                            long ttl)
                    : instanceName(instanceName)
                    , key(key)
                    , value(value)
                    , ttl(ttl) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MPUTIFABSENT";
                        command += SPACE + instanceName + SPACE;
                        char integerBuffer[20];
                        int integerBufferSize;

                        integerBufferSize = sprintf(integerBuffer, "%li", ttl);
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        command += "#2" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        integerBufferSize = sprintf(integerBuffer, "%d", value.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);
                        value.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (line.compare("OK #1"))
                            throw hazelcast::client::HazelcastException("unexpected header of putIfAbsent return");
                    };

                    void readSizeLine(std::string line) {
                        int returnSize = atoi(line.c_str());
                        resultSizes.push_back(returnSize);
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                        oldValue.readData(dataInput);
                    };

                    int nResults() {
                        return 1;
                    };

                    int resultSize(int i) {
                        return resultSizes[i];
                    };

                    hazelcast::client::serialization::Data get() {
                        return oldValue;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    hazelcast::client::serialization::Data value;
                    hazelcast::client::serialization::Data oldValue;
                    std::vector<int> resultSizes;
                    long ttl;
                    int returnSize;
                };

                class ReplaceIfSameCommand : public Command {
                public:

                    ReplaceIfSameCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key,
                            hazelcast::client::serialization::Data oldValue,
                            hazelcast::client::serialization::Data newValue)
                    : instanceName(instanceName)
                    , key(key)
                    , oldValue(oldValue)
                    , newValue(newValue) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MREPLACEIFSAME";
                        command += SPACE + instanceName + SPACE;
                        command += "#3" + NEWLINE;

                        char integerBuffer[20];
                        int integerBufferSize;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        integerBufferSize = sprintf(integerBuffer, "%d", oldValue.totalSize());
                        command.append(integerBuffer, integerBufferSize);
                        command += SPACE;

                        integerBufferSize = sprintf(integerBuffer, "%d", newValue.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);
                        oldValue.writeData(dataOutput);
                        newValue.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of replace if same return");

                        success = line.compare("OK true") ? false : true;
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };

                    bool get() {
                        return success;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    hazelcast::client::serialization::Data oldValue;
                    hazelcast::client::serialization::Data newValue;
                    bool success;
                };

                class EvictCommand : public Command {
                public:

                    EvictCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key)
                    : instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "MEVICT";
                        command += SPACE + instanceName + SPACE + "#1" + NEWLINE;
                        dataOutput.write(command.c_str(), command.length());

                        char integerBuffer[20];
                        int integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        dataOutput.write(integerBuffer, integerBufferSize);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of evict return");

                        success = line.compare("OK true") ? false : true;
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };

                    bool get() {
                        return success;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    bool success;
                };

                class KeySetCommand : public Command {
                public:

                    KeySetCommand(std::string instanceName)
                    : instanceName(instanceName) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        std::string command = "KEYSET";
                        command += SPACE + "map" + SPACE + instanceName + NEWLINE;
                        dataOutput.write(command.c_str(), command.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of('#');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK "))
                            throw hazelcast::client::HazelcastException("unexpected header of keySet return");
                        std::string sizeStr = line.substr(pos + 1, line.length() - pos);
                        nReturnedResults = std::atoi(sizeStr.c_str());
                    };

                    void readSizeLine(std::string line) {
                        int beg = 0, end = 0;
                        do {
                            end = line.find_first_of(" ", beg);

                            std::string keySize = line.substr(beg, end - beg);
                            int a = atoi(keySize.c_str());
                            keySizes.push_back(a);

                            beg = end;
                            beg++;

                        } while (end != std::string::npos);

                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                        using namespace hazelcast::client::serialization;
                        Data key;
                        key.readData(dataInput);
                        keys.push_back(key);
                    };

                    int nResults() {
                        return nReturnedResults;
                    };

                    int resultSize(int i) {
                        return keySizes[i];
                    };

                    std::vector<hazelcast::client::serialization::Data> get() {
                        return keys;
                    };
                private:
                    std::string instanceName;
                    int nReturnedResults;
                    std::vector<hazelcast::client::serialization::Data> keys;
                    std::vector<int> keySizes;
                };

                class LockCommand : public Command {
                public:

                    LockCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key) :
                    instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        char integerBuffer[20];
                        int integerBufferSize;

                        std::string command;
                        boost::hash<boost::thread::id> h;
                        int threadId = h(boost::this_thread::get_id());
                        integerBufferSize = sprintf(integerBuffer, "%d", threadId);
                        command.append(integerBuffer, integerBufferSize);

                        command += SPACE + "MLOCK" + SPACE + instanceName + SPACE + "#1" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of lock return");
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                };

                class IsLockedCommand : public Command {
                public:

                    IsLockedCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key) :
                    instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        char integerBuffer[20];
                        int integerBufferSize;

                        std::string command;
                        command += "MISLOCKED" + SPACE + instanceName + SPACE + "#1" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of isLocked return");

                        isLocked = line.compare("OK true") ? false : true;
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };

                    bool get() {
                        return isLocked;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    bool isLocked;
                };

                class TryLockCommand : public Command {
                public:

                    TryLockCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key,
                            long timeoutInMillis) :
                    instanceName(instanceName)
                    , key(key)
                    , timeoutInMillis(timeoutInMillis) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        char integerBuffer[20];
                        int integerBufferSize;

                        std::string command;
                        boost::hash<boost::thread::id> h;
                        int threadId = h(boost::this_thread::get_id());
                        integerBufferSize = sprintf(integerBuffer, "%d", threadId);
                        command.append(integerBuffer, integerBufferSize);


                        command += SPACE + "MTRYLOCK" + SPACE + instanceName + SPACE;

                        integerBufferSize = sprintf(integerBuffer, "%li", timeoutInMillis);
                        command.append(integerBuffer, integerBufferSize);

                        command += "#1" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of tryLock return");

                        isAcquired = line.compare("OK true") ? false : true;
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };

                    bool get() {
                        return isAcquired;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                    long timeoutInMillis;
                    bool isAcquired;
                };

                class UnlockCommand : public Command {
                public:

                    UnlockCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key) :
                    instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        char integerBuffer[20];
                        int integerBufferSize;

                        std::string command;
                        boost::hash<boost::thread::id> h;
                        int threadId = h(boost::this_thread::get_id());
                        integerBufferSize = sprintf(integerBuffer, "%d", threadId);
                        command.append(integerBuffer, integerBufferSize);

                        command += SPACE + "MUNLOCK" + SPACE + instanceName + SPACE + "#1" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        int pos = line.find_first_of(' ');
                        std::string ok = line.substr(0, pos);
                        if (ok.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of unlock return");
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                };

                class ForceUnlockCommand : public Command {
                public:

                    ForceUnlockCommand(std::string instanceName,
                            hazelcast::client::serialization::Data key) :
                    instanceName(instanceName)
                    , key(key) {
                    };

                    void writeCommand(hazelcast::client::serialization::DataOutput& dataOutput) {
                        char integerBuffer[20];
                        int integerBufferSize;

                        std::string command;
                        boost::hash<boost::thread::id> h;
                        int threadId = h(boost::this_thread::get_id());
                        integerBufferSize = sprintf(integerBuffer, "%d", threadId);
                        command.append(integerBuffer, integerBufferSize);

                        command += SPACE + "MFORCEUNLOCK" + SPACE + instanceName + SPACE + "#1" + NEWLINE;

                        integerBufferSize = sprintf(integerBuffer, "%d", key.totalSize());
                        command.append(integerBuffer, integerBufferSize);

                        command += NEWLINE;

                        dataOutput.write(command.c_str(), command.length());

                        key.writeData(dataOutput);

                        dataOutput.write(NEWLINE.c_str(), NEWLINE.length());
                    };

                    void readHeaderLine(std::string line) {
                        if (line.compare("OK"))
                            throw hazelcast::client::HazelcastException("unexpected header of force unlock return");
                    };

                    void readSizeLine(std::string line) {
                    };

                    void readResult(hazelcast::client::serialization::DataInput& dataInput) {
                    };

                    int nResults() {
                        return 0;
                    };

                    int resultSize(int i) {
                        return 0;
                    };
                private:
                    std::string instanceName;
                    hazelcast::client::serialization::Data key;
                };

            }
        }
    }
}

#endif /* HAZELCAST_MAP_COMMANDS */