#ifndef HAZELCAST_MULTI_MAP
#define HAZELCAST_MULTI_MAP

#include "collection/DestroyRequest.h"
#include "collection/CollectionProxyId.h"
#include "collection/PutRequest.h"
#include "collection/GetRequest.h"
#include "collection/RemoveRequest.h"
#include "collection/RemoveAllRequest.h"
#include "collection/KeySetRequest.h"
#include "collection/ValuesRequest.h"
#include "collection/EntrySetRequest.h"
#include "collection/PortableEntrySetResponse.h"
#include "collection/ContainsEntryRequest.h"
#include "collection/SizeRequest.h"
#include "collection/ClearRequest.h"
#include "collection/CountRequest.h"
#include "collection/AddEntryListenerRequest.h"
#include "collection/MultiMapLockRequest.h"
#include "collection/MultiMapUnlockRequest.h"
#include "impl/PortableCollection.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>

namespace hazelcast {
    namespace client {


        template<typename K, typename V>
        class MultiMap {
        public:

            MultiMap(const std::string& instanceName, spi::ClientContext& clientContext)
            : proxyId(instanceName, collection::CollectionProxyId::CollectionProxyType::MULTI_MAP)
            , context(clientContext) {

            };

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
                collection::PutRequest request(proxyId, keyData, valueData, -1, getThreadId());
                return invoke<V>(request, keyData);
            };

            /**
             * Returns the collection of values associated with the key.
             * <p/>
             * <p><b>Warning:</b></p>
             * <p>
             * This method uses <tt>hashCode</tt> and <tt>equals</tt> of binary form of
             * the <tt>key</tt>, not the actual implementations of <tt>hashCode</tt> and <tt>equals</tt>
             * defined in <tt>key</tt>'s class.
             * </p>
             * <p/>
             * <p><b>Warning-2:</b></p>
             * The collection is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
             *
             * @param key the key whose associated values are to be returned
             * @return the collection of the values associated with the key.
             */
            std::vector<V> get(const K& key) {
                serialization::Data keyData = toData(key);
                collection::GetRequest request(proxyId, keyData);
                return invoke< std::vector<V> >(request, keyData);
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
                collection::RemoveRequest request(proxyId, keyData, valueData, getThreadId());
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
             * The collection is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
             *
             * @param key the key of the entries to remove
             * @return the collection of removed values associated with the given key. Returned collection
             *         might be modifiable but it has no effect on the multimap
             */
            std::vector<V> remove(const K& key) {
                serialization::Data keyData = toData(key);
                collection::RemoveAllRequest request(proxyId, keyData, getThreadId());
                return toObjectCollection(invoke<impl::PortableCollection>(request, keyData));
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
                collection::KeySetRequest request(proxyId);
                return toObjectCollection(invoke<impl::PortableCollection>(request));
            };

            /**
             * Returns the collection of values in the multimap.
             * <p/>
             * <p><b>Warning:</b></p>
             * The collection is <b>NOT</b> backed by the map,
             * so changes to the map are <b>NOT</b> reflected in the collection, and vice-versa.
             *
             * @return the collection of values in the multimap. Returned collection might be modifiable
             *         but it has no effect on the multimap
             */
            std::vector<V> values() {
                collection::ValuesRequest request(proxyId);
                return toObjectCollection(invoke<impl::PortableCollection>(request));
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
            std::vector< std::pair<K, V> > entrySet() {
                collection::EntrySetRequest request(proxyId);
                collection::PortableEntrySetResponse result = invoke<collection::PortableEntrySetResponse>(request);
                const std::vector< std::pair<serialization::Data, serialization::Data> >& dataEntrySet = result.getEntrySet();
                std::vector< std::pair<K, V> > entrySet(dataEntrySet.size());
                for (int i = 0; i < dataEntrySet.size(); ++i) {
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
                collection::ContainsEntryRequest request (proxyId, keyData);
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
                collection::ContainsEntryRequest request (proxyId, valueData);
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
                serialization::Data keyData = toData(value);
                serialization::Data valueData = toData(value);
                collection::ContainsEntryRequest request (proxyId, keyData, valueData);
                return invoke<bool>(request, keyData);
            };

            /**
             * Returns the number of key-value pairs in the multimap.
             *
             * @return the number of key-value pairs in the multimap.
             */
            int size() {
                collection::SizeRequest request(proxyId);
                return invoke<int>(request);
            };

            /**
             * Clears the multimap. Removes all key-value pairs.
             */
            void clear() {
                collection::ClearRequest request(proxyId);
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
                collection::CountRequest request(proxyId, keyData);
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
                collection::AddEntryListenerRequest request(proxyId, includeValue);
                impl::EntryEventHandler<K, V, L> entryEventHandler(proxyId.getName() + proxyId.getKeyName(), context.getClusterService(), context.getSerializationService(), listener, includeValue);
                return context.getServerListenerService().template listen<map::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(proxyId.getName() + proxyId.getKeyName(), request, entryEventHandler);
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
                map::AddEntryListenerRequest request(proxyId, includeValue, keyData);
                impl::EntryEventHandler<K, V, L> entryEventHandler(proxyId.getName() + proxyId.getKeyName(), context.getClusterService(), context.getSerializationService(), listener, includeValue);
                return context.getServerListenerService().template listen<map::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(proxyId.getName() + proxyId.getKeyName(), request, keyData, entryEventHandler);
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
                return context.getServerListenerService().stopListening(proxyId.getName() + proxyId.getKeyName(), registrationId);
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
                collection::MultiMapLockRequest request(proxyId, keyData, getThreadId());
                invoke<bool>(request, keyData);
            };

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
                collection::MultiMapLockRequest request(proxyId, keyData, getThreadId(), -1, 0);
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
                collection::MultiMapLockRequest request(proxyId, keyData, getThreadId(), -1, timeoutInMillis);
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
                collection::MultiMapUnlockRequest request(proxyId, keyData, getThreadId());
                invoke<bool>(request, keyData);
            };

        private:
            std::vector<V> toObjectCollection(impl::PortableCollection result) {
                vector<serialization::Data> const & dataCollection = result.getCollection();
                std::vector<V> collection(dataCollection.size());
                for (int i = 0; i < dataCollection.size(); i++) {
                    collection[i] = toObject<V>(dataCollection[i]);
                }
                return collection;
            };

            void onDestroy() {
                collection::DestroyRequest request (proxyId);
                invoke<bool>(request);
            };

            template<typename T>
            serialization::Data toData(const T& object) {
                return context.getSerializationService().toData(object);
            };

            template<typename T>
            T toObject(const serialization::Data& data) {
                return context.getSerializationService().template toObject<T>(data);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request, serialization::Data&  keyData) {
                return context.getInvocationService().template invokeOnKeyOwner<Response>(request, keyData);
            };

            template<typename Response, typename Request>
            Response invoke(const Request& request) {
                return context.getInvocationService().template invokeOnRandomTarget<Response>(request);
            };

            int getThreadId() {
                return 1;
            };

            collection::CollectionProxyId proxyId;
            spi::ClientContext& context;
        };


    }
}

#endif /* HAZELCAST_MULTI_MAP */