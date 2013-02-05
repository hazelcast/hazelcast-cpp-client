#include "IMap.h"
#include "HazelcastClient.h"
#include "protocol/MapCommands.h"
#include <vector>
#include <map>

namespace hazelcast{ 
namespace client{
  
template<typename K,typename V>       
IMap<K,V>::IMap(std::string instanceName, HazelcastClient& hazelcastClient):instanceName(instanceName)
                                                                                 ,hazelcastClient(hazelcastClient){
    
};    

template<typename K,typename V>   
IMap<K,V>::IMap(const IMap<K,V>& rhs):instanceName(instanceName)
                                     ,hazelcastClient(rhs.hazelcastClient)
{   
};

template<typename K,typename V>   
IMap<K,V>::~IMap(){
    
};

template<typename K,typename V>   
bool IMap<K,V>::containsKey(K key){
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::ContainsKeyCommand command(instanceName,keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K,typename V>   
bool IMap<K,V>::containsValue(V value){
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::ContainsValueCommand command(instanceName,valueInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    return command.get();
};

template<typename K,typename V>   
V IMap<K,V>::get(K key){
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::GetCommand command(instanceName,keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    serialization::Data valueInBytes = command.get();
    return hazelcastClient.getSerializationService().toObject<V>(valueInBytes);
};

template<typename K,typename V>   
void IMap<K,V>::put(K key, V value){
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    serialization::Data valueInBytes = hazelcastClient.getSerializationService().toData(value);
    protocol::MapCommands::PutCommand command(instanceName,keyInBytes,valueInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
};

template<typename K,typename V>   
void IMap<K,V>::remove(K key){
    serialization::Data keyInBytes = hazelcastClient.getSerializationService().toData(key);
    protocol::MapCommands::RemoveCommand command(instanceName,keyInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    
};

template<typename K,typename V>   
std::string IMap<K,V>::getName() const{
    return instanceName;
};

template<typename K,typename V>
std::map<K,V> IMap<K,V>::getAll(std::set<K> keys){
    typedef std::vector< std::pair < serialization::Data , serialization::Data > >  MapEntrySet;
    typedef std::vector<hazelcast::client::serialization::Data> DataSet;
    int size = keys.size();
    DataSet keysInBytes(size);
  
    int i = 0;
    for(typename std::set<K>::iterator it = keys.begin() ; it != keys.end() ; it++ )
         keysInBytes[i++] = hazelcastClient.getSerializationService().toData(*it);
        
    protocol::MapCommands::GetAllCommand command(instanceName, keysInBytes);
    hazelcastClient.getCommandHandler().sendCommand(&command);
    DataSet resultKeys  = command.getKeys();
    DataSet resultValues = command.getValues();
    
    std::map<K,V> result;
    size = resultKeys.size();
    for(int i = 0 ; i < size ; i++){
        K key = hazelcastClient.getSerializationService().toObject<K>(resultKeys[i]);
        V value = hazelcastClient.getSerializationService().toObject<V>(resultValues[i]);
        result[key] = value;
    }
    return result;
};

}}