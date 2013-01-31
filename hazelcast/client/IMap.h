#ifndef HAZELCAST_IMAP
#define HAZELCAST_IMAP

#include <string>



namespace hazelcast{ 
namespace client{

class HazelcastClient;
    
template<typename K,typename V>    
class IMap{
public:
    IMap(std::string instanceName,HazelcastClient& hazelcastClient);
    IMap(const IMap& rhs);
    ~IMap();
      /**
     * {@inheritDoc}
     * <p/>
     * <p><b>Warning:</b></p>
     * <p>
     * This method returns a clone of original value, modifying the returned value does not change
     * the actual value in the map. One should put modified value back to make changes visible to all nodes.
     * <pre>
     *      V value = map.get(key);
     *      value.updateSomeProperty();
     *      map.put(key, value);
     * </pre>
     * </p>
     * <p/>
     * <p><b>Warning-2:</b></p>
     * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
     * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
     * defined in <tt>key</tt>'s class.
     * <p/>
     * <p><b>Warning-3:</b></p>
     * <p>
     * If <tt>cache-value</tt> is true (default is true), this method returns a clone of original value
     * but also caches that value for fast access in local. Modifications done to this cached value without
     * putting it back to map will be visible to only local node, not entire cluster,
     * successive <tt>get</tt> calls will return the same cached value.
     * To reflect modifications to distributed map, one should put modified value back into map.
     * </p>
     */
    V get(K key);

    /**
     * {@inheritDoc}
     * <p/>
     * <p><b>Warning:</b></p>
     * <p>
     * This method returns a clone of previous value, not the original (identically equal) value
     * previously put into map.
     * </p>
     * <p/>
     * <p><b>Warning-2:</b></p>
     * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
     * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
     * defined in <tt>key</tt>'s class.
     */
    V put(K key, V value);
private:
    std::string instanceName;    
    HazelcastClient& hazelcastClient;
};

    
}}

#endif /* HAZELCAST_IMAP */