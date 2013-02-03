#include "IMap.h"
#include "HazelcastClient.h"
#include "protocol/MapCommands.h"

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
V IMap<K,V>::put(K key, V value){
    serialization::Data keyInBytes = hazelcastClient.serializationService.toData(key);
    serialization::Data valueInBytes = hazelcastClient.serializationService.toData(value);
    protocol::MapCommands::PutCommand command(instanceName,keyInBytes,valueInBytes);
    hazelcastClient.commandHandler.sendCommand(&command);
    return hazelcastClient.serializationService.toObject<V>(command.get());
};

template<typename K,typename V>   
V IMap<K,V>::get(K key){
    serialization::Data keyInBytes = hazelcastClient.serializationService.toData(key);
    protocol::MapCommands::GetCommand command(instanceName,keyInBytes);
    hazelcastClient.commandHandler.sendCommand(&command);
    serialization::Data valueInBytes = command.get();
    return hazelcastClient.serializationService.toObject<V>(valueInBytes);
};

}}