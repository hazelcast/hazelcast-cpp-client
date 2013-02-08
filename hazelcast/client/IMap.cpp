#include "IMap.h"
#include "HazelcastClient.h"
#include "protocol/MapCommands.h"
#include <vector>
#include <map>

namespace hazelcast {
namespace client {

template<typename K, typename V>
IMap<K, V>::IMap(std::string instanceName, HazelcastClient& hazelcastClient) : instanceName(instanceName)
, hazelcastClient(hazelcastClient) {

};

template<typename K, typename V>
IMap<K, V>::IMap(const IMap<K, V>& rhs) : instanceName(rhs.instanceName)
, hazelcastClient(rhs.hazelcastClient) {
};

template<typename K, typename V>
IMap<K, V>::~IMap() {

};

template<typename K, typename V>
bool IMap<K, V>::containsKey(K key) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::ContainsKeyCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
bool IMap<K, V>::containsValue(V value) {
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::ContainsValueCommand command(instanceName, valueInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
V IMap<K, V>::get(K key) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::GetCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    if (command.nResults() == 1) {
        serialization::Data valueInBytes = command.get();
        return hazelcastClient.getSerializationService().template toObject<V > (valueInBytes);
    } else
        return V();
};

template<typename K, typename V>
void IMap<K, V>::put(K key, V value) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::PutCommand command(instanceName, keyInBytes, valueInBytes, 0);
    hazelcastClient.getCommandHandler().sendCommand(&command);
};

template<typename K, typename V>
void IMap<K, V>::flush() {
    protocol::MapCommands::FlushCommand command(instanceName);
    hazelcastClient.getCommandHandler().sendCommand(&command);
};

template<typename K, typename V>
void IMap<K, V>::remove(K key) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::RemoveCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);

};

template<typename K, typename V>
std::string IMap<K, V>::getName() const {
    return instanceName;
};

template<typename K, typename V>
std::map<K, V> IMap<K, V>::getAll(std::set<K> keys) {
    typedef std::vector<hazelcast::client::serialization::Data> DataSet;
    int size = keys.size();
    DataSet keysInBytes(size);

    int i = 0;
    for (typename std::set<K>::iterator it = keys.begin(); it != keys.end(); it++)
        keysInBytes[i++] = hazelcastClient.getSerializationService().toData(*it);

    protocol::MapCommands::GetAllCommand command(instanceName, keysInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    DataSet resultKeys = command.getKeys();
    DataSet resultValues = command.getValues();

    std::map<K, V> result;
    size = resultKeys.size();
    for (int i = 0; i < size; i++) {
        K key = hazelcastClient.getSerializationService().template toObject<K > (resultKeys[i]);
        V value = hazelcastClient.getSerializationService().template toObject<V > (resultValues[i]);
        result[key] = value;
    }
    return result;
};

/**
 * 
 * @param key
 * @param timeoutInMillis
 * @return true if key is removed from map in specified time (timeoutInMillis).
 */
template<typename K, typename V>
bool IMap<K, V>::tryRemove(K key, long timeoutInMillis) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::TryRemoveCommand command(instanceName, keyInBytes, timeoutInMillis);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
bool IMap<K, V>::tryPut(K key, V value, long timeoutInMillis) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::TryPutCommand command(instanceName, keyInBytes, valueInBytes, timeoutInMillis);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
void IMap<K, V>::put(K key, V value, long ttl) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::PutCommand command(instanceName, keyInBytes, valueInBytes, ttl);
    hazelcastClient.getCommandHandler().sendCommand(&command);
};

template<typename K, typename V>
void IMap<K, V>::putTransient(K key, V value, long ttl) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::PutTransientCommand command(instanceName, keyInBytes, valueInBytes, ttl);
    hazelcastClient.getCommandHandler().sendCommand(&command);

};

/*
template<typename K, typename V>
V IMap<K,V>::putIfAbsent(K key, V value){
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::PutIfAbsentCommand command(instanceName,keyInBytes,valueInBytes, 0);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return hazelcastClient.getSerializationService().toObject<K>(command.get());
};

template<typename K, typename V>
V IMap<K,V>::putIfAbsent(K key, V value, long ttl){
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::PutIfAbsentCommand command(instanceName,keyInBytes,valueInBytes, ttl);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return hazelcastClient.getSerializationService().toObject<K>(command.get());
};*/

template<typename K, typename V>
bool IMap<K, V>::replace(K key, V oldValue, V newValue) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data oldValueInBytes = hazelcastClient.getSerializationService().toData(oldValue);
    serialization::Data newValueInBytes = hazelcastClient.getSerializationService().toData(newValue);
    protocol::MapCommands::ReplaceIfSameCommand command(instanceName, keyInBytes, oldValueInBytes, newValueInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
std::pair<K, V> IMap<K, V>::getEntry(K key) {
    V value = get(key);
    std::pair<K, V> mapEntry;
    mapEntry.first = key;
    mapEntry.second = value;
    return mapEntry;
};

template<typename K, typename V>
bool IMap<K, V>::evict(K key) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::EvictCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
std::set<K> IMap<K, V>::keySet() {
    protocol::MapCommands::KeySetCommand command(instanceName);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    std::set<K> resultSet;
    std::vector<hazelcast::client::serialization::Data> resultDataVector = command.get();
    int size = resultDataVector.size();
    for (int i = 0; i < size; i++) {
        K key = hazelcastClient.getSerializationService().template toObject<K > (resultDataVector[i]);
        resultSet.insert(key);
    }
    return resultSet;
};

template<typename K, typename V>
std::vector<V> IMap<K, V>::values() {
    std::set<K> allKeys = keySet();
    std::map<K, V> allMap = getAll(allKeys);
    std::vector<V> values;
    for (typename std::map<K, V>::iterator it = allMap.begin(); it != allMap.end(); it++)
        values.push_back(it->second);
    return values;
};

template<typename K, typename V>
std::vector< std::pair < K, V > > IMap<K, V>::entrySet() {
    std::set<K> allKeys = keySet();
    std::map<K, V> allMap = getAll(allKeys);
    std::vector< std::pair < K, V > > mapEntries;
    for (typename std::map<K, V>::iterator it = allMap.begin(); it != allMap.end(); it++)
        mapEntries.push_back(pair< K, V > (it->first, it->second));
    return mapEntries;
};

template<typename K, typename V>
void IMap<K, V>::lock(K key) throw (std::domain_error) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::LockCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
};

template<typename K, typename V>
bool IMap<K, V>::isLocked(K key) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::IsLockedCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
bool IMap<K, V>::tryLock(K key, long timeoutInMillis) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::TryLockCommand command(instanceName, keyInBytes, timeoutInMillis);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K, typename V>
void IMap<K, V>::unlock(K key) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::UnlockCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
};

template<typename K, typename V>
void IMap<K, V>::forceunlock(K key) {
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::ForceUnlockCommand command(instanceName, keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
};

}}