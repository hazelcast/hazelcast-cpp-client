#ifndef HAZELCAST_IMAP
#define HAZELCAST_IMAP

#include <string>
#include <map>
#include <set>

namespace hazelcast{ 
namespace client{

class HazelcastClient;
    
template<typename K,typename V>    
class IMap{
public:
    IMap(std::string instanceName,HazelcastClient& hazelcastClient);
    IMap(const IMap& rhs);
    ~IMap();

    bool containsKey(K key);
    bool containsValue(V value);
    V get(K key);
    void put(K key, V value);
    void remove(K key);
    void flush();
    std::string getName() const;
    std::map< K , V > getAll(std::set<K> keys);
    bool tryRemove(K key, long timeoutInMillis);
    bool tryPut(K key, V value, long timeoutInMillis);
    void put(K key, V value, long ttl);
    void putTransient(K key, V value, long ttl);
    
private:
    std::string instanceName;    
    HazelcastClient& hazelcastClient;
};

    
}}

#endif /* HAZELCAST_IMAP */