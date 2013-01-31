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
    serialization::Data result = hazelcastClient.commandHandler.sendCommand(&command);
    return hazelcastClient.serializationService.toObject<V>(result);
};

template<typename K,typename V>   
V IMap<K,V>::get(K key){
    
};

}}