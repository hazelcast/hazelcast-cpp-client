//#ifndef HAZELCAST_IMAP
//#define HAZELCAST_IMAP
//
//#include "ClientService.h"
//#include <string>
//#include <map>
//#include <set>
//#include <vector>
//#include <stdexcept>
//
//namespace hazelcast {
//    namespace client {
//
//        class impl::ClientService;
//
//        template<typename K, typename V>
//        class IMap {
//        public:
//
//            IMap(std::string instanceName, impl::ClientService& clientService) : instanceName(instanceName)
//            , clientService(clientService) {
//
//            };
//
//            IMap(const IMap& rhs) : instanceName(rhs.instanceName)
//            , clientService(rhs.clientService) {
//            };
//
//            ~IMap() {
//
//            };
//
//            std::string getName() const {
//                return instanceName;
//            };
//
//            bool containsKey(K key) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::ContainsKeyCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            bool containsValue(V value) {
////                serialization::Data valueInBytes = clientService.getSerializationService().toData(value);
////                protocol::MapCommands::ContainsValueCommand command(instanceName, valueInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            V get(K key) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::GetCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////                if (command.nResults() == 1) {
////                    serialization::Data valueInBytes = command.get();
////                    return clientService.getSerializationService().template toObject<V >(valueInBytes);
////                } else
////                    return V();
//            };
//
//            void put(K key, V value) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                serialization::Data valueInBytes = clientService.getSerializationService().toData(value);
////                protocol::MapCommands::PutCommand command(instanceName, keyInBytes, valueInBytes, 0);
////                clientService.getCommandHandler().sendCommand(&command);
//            };
//
//            void remove(K key) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::RemoveCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////
//            };
//
//            void flush() {
////                protocol::MapCommands::FlushCommand command(instanceName);
////                clientService.getCommandHandler().sendCommand(&command);
//            };
//
//            std::map< K, V > getAll(std::set<K> keys) {
////                typedef std::vector<hazelcast::client::serialization::Data> DataSet;
////                int size = keys.size();
////                DataSet keysInBytes(size);
////
////                int i = 0;
////                for (typename std::set<K>::iterator it = keys.begin(); it != keys.end(); it++)
////                    keysInBytes[i++] = clientService.getSerializationService().toData(*it);
////
////                protocol::MapCommands::GetAllCommand command(instanceName, keysInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////                DataSet resultKeys = command.getKeys();
////                DataSet resultValues = command.getValues();
////
////                std::map<K, V> result;
////                size = resultKeys.size();
////                for (int i = 0; i < size; i++) {
////                    K key = clientService.getSerializationService().template toObject<K >(resultKeys[i]);
////                    V value = clientService.getSerializationService().template toObject<V >(resultValues[i]);
////                    result[key] = value;
////                }
////                return result;
//            };
//
//            bool tryRemove(K key, long timeoutInMillis) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::TryRemoveCommand command(instanceName, keyInBytes, timeoutInMillis);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            bool tryPut(K key, V value, long timeoutInMillis) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                serialization::Data valueInBytes = clientService.getSerializationService().toData(value);
////                protocol::MapCommands::TryPutCommand command(instanceName, keyInBytes, valueInBytes, timeoutInMillis);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            void put(K key, V value, long ttl) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                serialization::Data valueInBytes = clientService.getSerializationService().toData(value);
////                protocol::MapCommands::PutCommand command(instanceName, keyInBytes, valueInBytes, ttl);
////                clientService.getCommandHandler().sendCommand(&command);
//            };
//
//            void putTransient(K key, V value, long ttl) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                serialization::Data valueInBytes = clientService.getSerializationService().toData(value);
////                protocol::MapCommands::PutTransientCommand command(instanceName, keyInBytes, valueInBytes, ttl);
////                clientService.getCommandHandler().sendCommand(&command);
//
//            };
//
//            //    V putIfAbsent(K key ,V value){
//            //    serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
//            //    serialization::Data valueInBytes = clientService.getSerializationService().toData(value);
//            //    protocol::MapCommands::PutIfAbsentCommand command(instanceName,keyInBytes,valueInBytes, 0);
//            //    clientService.getCommandHandler().sendCommand(&command);
//            //    return clientService.getSerializationService().toObject<K>(command.get());
//            //};
//            //    V putIfAbsent(K key ,V value,long ttl){
//            //    serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
//            //    serialization::Data valueInBytes = clientService.getSerializationService().toData(value);
//            //    protocol::MapCommands::PutIfAbsentCommand command(instanceName,keyInBytes,valueInBytes, ttl);
//            //    clientService.getCommandHandler().sendCommand(&command);
//            //    return clientService.getSerializationService().toObject<K>(command.get());
//            //};
//
//            bool replace(K key, V oldValue, V newValue) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                serialization::Data oldValueInBytes = clientService.getSerializationService().toData(oldValue);
////                serialization::Data newValueInBytes = clientService.getSerializationService().toData(newValue);
////                protocol::MapCommands::ReplaceIfSameCommand command(instanceName, keyInBytes, oldValueInBytes, newValueInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            //TODO V replace(K,V)
//            //TODO set
//            //TODO locks and listeners
//
//            std::pair<K, V> getEntry(K key) {
////                V value = get(key);
////                std::pair<K, V> mapEntry;
////                mapEntry.first = key;
////                mapEntry.second = value;
////                return mapEntry;
//            };
//
//            //predicates ? no support on protocol TODO
//
//            bool evict(K key) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::EvictCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            std::set<K> keySet() {
////                protocol::MapCommands::KeySetCommand command(instanceName);
////                clientService.getCommandHandler().sendCommand(&command);
////                std::set<K> resultSet;
////                std::vector<hazelcast::client::serialization::Data> resultDataVector = command.get();
////                int size = resultDataVector.size();
////                for (int i = 0; i < size; i++) {
////                    K key = clientService.getSerializationService().template toObject<K >(resultDataVector[i]);
////                    resultSet.insert(key);
////                }
////                return resultSet;
//            };
//
//            std::vector<V> values() {
////                std::set<K> allKeys = keySet();
////                std::map<K, V> allMap = getAll(allKeys);
////                std::vector<V> values;
////                for (typename std::map<K, V>::iterator it = allMap.begin(); it != allMap.end(); it++)
////                    values.push_back(it->second);
////                return values;
//            };
//
//            std::vector< std::pair<K, V> > entrySet() {
////                std::set<K> allKeys = keySet();
////                std::map<K, V> allMap = getAll(allKeys);
////                std::vector< std::pair < K, V > > mapEntries;
////                for (typename std::map<K, V>::iterator it = allMap.begin(); it != allMap.end(); it++)
////                    mapEntries.push_back(pair< K, V >(it->first, it->second));
////                return mapEntries;
//            };
//
//            void lock(K key) throw (hazelcast::client::HazelcastException) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::LockCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
//            };
//
//            bool isLocked(K key) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::IsLockedCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            bool tryLock(K key, long timeoutInMillis) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::TryLockCommand command(instanceName, keyInBytes, timeoutInMillis);
////                clientService.getCommandHandler().sendCommand(&command);
////                return command.get();
//            };
//
//            void unlock(K key) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::UnlockCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
//            };
//
//            void forceUnlock(K key) {
////                serialization::Data keyInBytes = clientService.getSerializationService().toData(key);
////                protocol::MapCommands::ForceUnlockCommand command(instanceName, keyInBytes);
////                clientService.getCommandHandler().sendCommand(&command);
//            };
//
//        private:
//            std::string instanceName;
//            impl::ClientService& clientService;
//        };
//    }
//}
//
//#endif /* HAZELCAST_IMAP */