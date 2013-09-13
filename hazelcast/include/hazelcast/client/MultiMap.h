#ifndef HAZELCAST_MULTI_MAP
#define HAZELCAST_MULTI_MAP

#include "multimap/PutRequest.h"
#include "multimap/RemoveRequest.h"
#include "multimap/RemoveAllRequest.h"
#include "multimap/KeySetRequest.h"
#include "multimap/ValuesRequest.h"
#include "multimap/EntrySetRequest.h"
#include "multimap/ContainsEntryRequest.h"
#include "multimap/SizeRequest.h"
#include "multimap/ClearRequest.h"
#include "multimap/CountRequest.h"
#include "multimap/AddEntryListenerRequest.h"
#include "multimap/GetAllRequest.h"
#include "multimap/MultiMapLockRequest.h"
#include "multimap/MultiMapUnlockRequest.h"
#include "hazelcast/client/spi/DistributedObjectListenerService.h"
#include "PortableCollection.h"
#include "MultiMapDestroyRequest.h"
#include "MultiMapIsLockedRequest.h"
#include "PortableEntrySetResponse.h"
#include "ClientContext.h"
#include "EntryEventHandler.h"
#include "ServerListenerService.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>

namespace hazelcast {
    namespace client {


        template<typename K, typename V>
        class MultiMap {
            friend class HazelcastClient;

        public:
            /**
             * Stores a key-value pair in the multimap.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param key   the key to be stored
             * @param value the value to be stored
             * @return true if size of the multimap is increased, false if the multimap
             *         already contains the key-value pair.
             */
            bool put(const K& key, const V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                multimap::PutRequest request(name, keyData, valueData, -1, util::getThreadId());
                return invoke<bool>(request, keyData);
            };

            /**
             * Returns the multimap of values associated with the key.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             * <p/>
             * <p><b>Warning-2:</b></p>
             * The multimap is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the multimap, and vice-versa.
             *
             * @param key the key whose associated values are to be returned
             * @return the multimap of the values associated with the key.
             */
            std::vector<V> get(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::GetAllRequest request(name, keyData);
                return toObjectCollection(invoke < impl::PortableCollection >(request, keyData));
            };

            /**
             * Removes the given key value pair from the multimap.
             * <p/>
             * <p><b>Warning:</b></p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             *
             * @param key   the key of the entry to remove
             * @param value the value of the entry to remove
             * @return true if the size of the multimap changed after the remove operation, false otherwise.
             */
            bool remove(const K& key, const V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                multimap::RemoveRequest request(name, keyData, valueData, util::getThreadId());
                return invoke<bool>(request, keyData);
            };

            /**
             * Removes all the entries with the given key.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             * <p/>
             * <p><b>Warning-2:</b></p>
             * The multimap is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the multimap, and vice-versa.
             *
             * @param key the key of the entries to remove
             * @return the multimap of removed values associated with the given key. Returned multimap
             *         might be modifiable but it has no effect on the multimap
             */
            std::vector<V> remove(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::RemoveAllRequest request(name, keyData, util::getThreadId());
                return toObjectCollection(invoke < impl::PortableCollection >(request, keyData));
            };

            /**
             * Returns the set of keys in the multimap.
             * <p/>
             * <p><b>Warning:</b></p>
             * The set is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the set, and vice-versa.
             *
             * @return the set of keys in the multimap. Returned set might be modifiable
             *         but it has no effect on the multimap
             */
            std::vector<K> keySet() {
                multimap::KeySetRequest request(name);
                return toObjectCollection(invoke < impl::PortableCollection >(request));
            };

            /**
             * Returns the multimap of values in the multimap.
             * <p/>
             * <p><b>Warning:</b></p>
             * The multimap is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the multimap, and vice-versa.
             *
             * @return the multimap of values in the multimap. Returned multimap might be modifiable
             *         but it has no effect on the multimap
             */
            std::vector<V> values() {
                multimap::ValuesRequest request(name);
                return toObjectCollection(invoke < impl::PortableCollection >(request));
            };

            /**
             * Returns the set of key-value pairs in the multimap.
             * <p/>
             * <p><b>Warning:</b></p>
             * The set is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the set, and vice-versa.
             *
             * @return the set of key-value pairs in the multimap. Returned set might be modifiable
             *         but it has no effect on the multimap
             */
            std::vector<std::pair<K, V> > entrySet() {
                multimap::EntrySetRequest request(name);
                multimap::PortableEntrySetResponse result = invoke < multimap::PortableEntrySetResponse >(request);
                std::vector < std::pair<serialization::Data, serialization::Data> > const & dataEntrySet = result.getEntrySet();
                std::vector<std::pair<K, V> > entrySet(dataEntrySet.size());
                for (int i = 0; i < dataEntrySet.size(); i++) {
                    entrySet[i].first = toObject<K>(dataEntrySet[i].first);
                    entrySet[i].second = toObject<V>(dataEntrySet[i].second);
                }
                return entrySet;
            };

            /**
             * Returns whether the multimap contains an entry with the key.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param key the key whose existence is checked.
             * @return true if the multimap contains an entry with the key, false otherwise.
             */
            bool containsKey(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::ContainsEntryRequest request (keyData, name);
                return invoke<bool>(request, keyData);
            };

            /**
             * Returns whether the multimap contains an entry with the value.
             * <p/>
             *
             * @param value the value whose existence is checked.
             * @return true if the multimap contains an entry with the value, false otherwise.
             */
            bool containsValue(const V& value) {
                serialization::Data valueData = toData(value);
                multimap::ContainsEntryRequest request (name, valueData);
                return invoke<bool>(request, valueData);
            };

            /**
             * Returns whether the multimap contains the given key-value pair.
             * <p/>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             *
             * @param key   the key whose existence is checked.
             * @param value the value whose existence is checked.
             * @return true if the multimap contains the key-value pair, false otherwise.
             */
            bool containsEntry(const K& key, const V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                multimap::ContainsEntryRequest request (keyData, name, valueData);
                return invoke<bool>(request, keyData);
            };

            /**
             * Returns the number of key-value pairs in the multimap.
             *
             * @return the number of key-value pairs in the multimap.
             */
            int size() {
                multimap::SizeRequest request(name);
                return invoke<int>(request);
            };

            /**
             * Clears the multimap. Removes all key-value pairs.
             */
            void clear() {
                multimap::ClearRequest request(name);
                invoke<bool>(request);
            };

            /**
             * Returns number of values matching to given key in the multimap.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param key the key whose values count are to be returned
             * @return number of values matching to given key in the multimap.
             */
            int valueCount(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::CountRequest request(name, keyData);
                return invoke<int>(request, keyData);
            };

            /**
             * Adds an entry listener for this multimap. Listener will get notified
             * for all multimap add/remove/update/evict events.
             *
             * @param listener     entry listener
             * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
             *                     contain the value.
             * @return returns registration id.
             */
            template < typename L>
            long addEntryListener(L& listener, bool includeValue) {
                multimap::AddEntryListenerRequest request(name, includeValue);
                impl::EntryEventHandler<K, V, L> entryEventHandler(name, context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return context->getServerListenerService().template listen<multimap::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(request, entryEventHandler);
            };

            /**
             * Adds the specified entry listener for the specified key.
             * The listener will get notified for all
             * add/remove/update/evict events of the specified key only.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param listener     entry listener
             * @param key          the key to listen
             * @param includeValue <tt>true</tt> if <tt>EntryEvent</tt> should
             *                     contain the value.
             * @return returns registration id.
             */
            template < typename L>
            long addEntryListener(L& listener, const K& key, bool includeValue) {
                serialization::Data keyData = toData(key);
                serialization::Data cloneData = keyData.clone();
                multimap::AddEntryListenerRequest request(name, keyData, includeValue);
                impl::EntryEventHandler<K, V, L> entryEventHandler(name, context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return context->getServerListenerService().template listen<multimap::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(request, cloneData, entryEventHandler);
            };

            /**
            * Removes the specified entry listener
            * Returns silently if there is no such listener added before.
            *
            * @param registrationId Id of listener registration
            *
            * @return true if registration is removed, false otherwise
            */
            bool removeEntryListener(long registrationId) {
                return context->getServerListenerService().stopListening(registrationId);
            };

            /**
             * Acquires the lock for the specified key.
             * <p>If the lock is not available then
             * the current thread becomes disabled for thread scheduling
             * purposes and lies dormant until the lock has been acquired.
             * <p/>
             * Scope of the lock is this multimap only.
             * Acquired lock is only for the key in this multimap.
             * <p/>
             * Locks are re-entrant so if the key is locked N times then
             * it should be unlocked N times before another thread can acquire it.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param key key to lock.
             */
            void lock(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::MultiMapLockRequest request(name, keyData, util::getThreadId());
                invoke<bool>(request, keyData);
            };

            /**
            * Acquires the lock for the specified key for the specified lease time.
            * <p>After lease time, lock will be released..
            * <p/>
            * <p>If the lock is not available then
            * the current thread becomes disabled for thread scheduling
            * purposes and lies dormant until the lock has been acquired.
            * <p/>
            * Scope of the lock is this map only.
            * Acquired lock is only for the key in this map.
            * <p/>
            * Locks are re-entrant so if the key is locked N times then
            * it should be unlocked N times before another thread can acquire it.
            * <p/>
            * <p><b>Warning:</b></p>
            * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
            * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
            * defined in <tt>key</tt>'s class.
            *
            * @param key key to lock.
            * @param leaseTime time to wait before releasing the lock.
            * @param timeUnit unit of time to specify lease time.
            */
            void lock(const K& key, long leaseTimeInMillis) {
                serialization::Data keyData = toData(key);
                multimap::MultiMapLockRequest request(name, keyData, util::getThreadId(), leaseTimeInMillis, -1);
                invoke<bool>(request, keyData);
            }

            /**
             * Checks the lock for the specified key.
             * <p>If the lock is acquired then returns true, else false.
             * <p/>
             * <p><b>Warning:</b></p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             *
             * @param key key to lock to be checked.
             * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
             */
            bool isLocked(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::MultiMapIsLockedRequest request(name, keyData);
                return invoke<bool>(request, keyData);
            }

            /**
             * Tries to acquire the lock for the specified key.
             * <p>If the lock is not available then the current thread
             * doesn't wait and returns false immediately.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param key key to lock.
             * @return <tt>true</tt> if lock is acquired, <tt>false</tt> otherwise.
             */
            bool tryLock(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::MultiMapLockRequest request(name, keyData, util::getThreadId(), -1, 0);
                return invoke<bool>(request, keyData);
            };

            /**
             * Tries to acquire the lock for the specified key.
             * <p>If the lock is not available then
             * the current thread becomes disabled for thread scheduling
             * purposes and lies dormant until one of two things happens:
             * <ul>
             * <li>The lock is acquired by the current thread; or
             * <li>The specified waiting time elapses
             * </ul>
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param time     the maximum time to wait for the lock
             * @param timeunit the time unit of the <tt>time</tt> argument.
             * @return <tt>true</tt> if the lock was acquired and <tt>false</tt>
             *         if the waiting time elapsed before the lock was acquired.
             */
            bool tryLock(const K& key, long timeoutInMillis) {
                serialization::Data keyData = toData(key);
                multimap::MultiMapLockRequest request(name, keyData, util::getThreadId(), -1, timeoutInMillis);
                return invoke<bool>(request, keyData);
            };

            /**
             * Releases the lock for the specified key. It never blocks and
             * returns immediately.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             *
             * @param key key to lock.
             */
            void unlock(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::MultiMapUnlockRequest request(name, keyData, util::getThreadId());
                invoke<bool>(request, keyData);
            };

            /**
            * Releases the lock for the specified key regardless of the lock owner.
            * It always successfully unlocks the key, never blocks
            * and returns immediately.
            * <p/>
            * <p><b>Warning:</b></p>
            * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
            * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
            * defined in <tt>key</tt>'s class.
            *
            * @param key key to lock.
            */
            void forceUnlock(const K& key) {
                serialization::Data keyData = toData(key);
                multimap::MultiMapUnlockRequest request(name, keyData, util::getThreadId(), true);
                invoke<bool>(request, keyData);
            }

            /**
            * Destroys this object cluster-wide.
            * Clears and releases all resources for this object.
            */
            void destroy() {
                multimap::MultiMapDestroyRequest request (name);
                invoke<bool>(request);
                context->getDistributedObjectListenerService().removeDistributedObject(name);
            };
        private:
            std::vector<V> toObjectCollection(impl::PortableCollection result) {
                std::vector<serialization::Data> const & dataCollection = result.getCollection();
                std::vector<V> multimap(dataCollection.size());
                for (int i = 0; i < dataCollection.size(); i++) {
                    multimap[i] = toObject<V>(dataCollection[i]);
                }
                return multimap;
            };


            template<typename T>
            serialization::Data toData(const T& object) {
                return context->getSerializationService().toData<T>(&object);
            };

            template<typename T>
            T toObject(const serialization::Data& data) {
                return context->getSerializationService().template toObject<T>(data);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request, serialization::Data&  keyData) {
                return context->getInvocationService().template invokeOnKeyOwner<Response>(request, keyData);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return context->getInvocationService().template invokeOnRandomTarget<Response>(request);
            };

            MultiMap() {

            };

            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                context = clientContext;
                name = instanceName;
            };

            std::string name;
            spi::ClientContext *context;
        };


    }
}

#endif /* HAZELCAST_MULTI_MAP */