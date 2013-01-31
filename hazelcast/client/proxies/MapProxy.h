#ifndef HAZELCAST_MAP_PROXY
#define HAZELCAST_MAP_PROXY

#include "../IMap.h"

namespace hazelcast{ 
namespace client{
namespace proxies{
     
 
template<typename K,typename V>    
class MapProxy : public IMap<K,V>{
public:
    V get(K key);
    V put(K key, V value);
        
};
    
}}}

#endif /* HAZELCAST_MAP_PROXY */