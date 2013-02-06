#ifndef HAZELCAST_IMAP
#define HAZELCAST_IMAP

#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>

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
//    V putIfAbsent(K key ,V value);
//    V putIfAbsent(K key ,V value,long ttl);
    bool replace(K key, V oldValue, V newValue);
    //TODO V replace(K,V)
    //TODO set
    //TODO locks and listeners
    std::pair<K,V> getEntry(K key);
    //predicates ? no support on protocol TODO
    bool evict(K key);
    std::set<K> keySet();
    std::vector<V> values();
    std::vector< std::pair<K, V> > entrySet();
    void lock(K key) throw(std::domain_error);
    bool isLocked(K key);
    bool tryLock(K key, long timeoutInMillis);
    void unlock(K key);
    void forceunlock(K key);
private:
    std::string instanceName;    
    HazelcastClient& hazelcastClient;
};

    
}}

#endif /* HAZELCAST_IMAP */