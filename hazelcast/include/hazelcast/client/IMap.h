#ifndef HAZELCAST_IMAP
#define HAZELCAST_IMAP

#include "IException.h"
#include "serialization/Data.h"
#include "spi/ClientContext.h"
#include "spi/InvocationService.h"
#include "spi/ServerListenerService.h"
#include "impl/MapKeySet.h"
#include "impl/MapEntrySet.h"
#include "impl/MapValueCollection.h"
#include "map/GetRequest.h"
#include "map/PutRequest.h"
#include "map/RemoveRequest.h"
#include "map/ContainsKeyRequest.h"
#include "map/ContainsValueRequest.h"
#include "map/FlushRequest.h"
#include "map/RemoveIfSameRequest.h"
#include "map/DeleteRequest.h"
#include "map/TryRemoveRequest.h"
#include "map/TryPutRequest.h"
#include "map/PutTransientRequest.h"
#include "map/ReplaceIfSameRequest.h"
#include "map/ReplaceRequest.h"
#include "map/SetRequest.h"
#include "map/LockRequest.h"
#include "map/IsLockedRequest.h"
#include "map/UnlockRequest.h"
#include "map/GetEntryViewRequest.h"
#include "map/EvictRequest.h"
#include "map/KeySetRequest.h"
#include "map/GetAllRequest.h"
#include "map/EntrySetRequest.h"
#include "map/ValuesRequest.h"
#include "map/AddIndexRequest.h"
#include "map/SizeRequest.h"
#include "map/DestroyRequest.h"
#include "map/ClearRequest.h"
#include "map/PutAllRequest.h"
#include "map/QueryRequest.h"
#include "map/QueryDataResultStream.h"
#include "map/EntryView.h"
#include "map/AddEntryListenerRequest.h"
#include "impl/EntryListener.h"
#include "impl/EntryEventHandler.h"
#include "impl/PortableEntryEvent.h"
#include "serialization/SerializationService.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>

namespace hazelcast {
    namespace client {

        template<typename K, typename V>
        class IMap {
        public:

            void init(const std::string& instanceName, spi::ClientContext *clientContext) {
                this->context = clientContext;
                this->instanceName = instanceName;
            };

            bool containsKey(const K& key) {
                serialization::Data keyData = toData(key);
                map::ContainsKeyRequest request(instanceName, keyData);
                return invoke<bool>(request, keyData);
            };

            bool containsValue(const V& value) {
                serialization::Data valueData = toData(value);
                map::ContainsValueRequest request(instanceName, valueData);
                return invoke<bool>(request, valueData);
            };

            V get(const K& key) {
                serialization::Data keyData = toData(key);
                map::GetRequest request(instanceName, keyData);
                return invoke<V>(request, keyData);
            };

            V put(const K& key, V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::PutRequest request(instanceName, keyData, valueData, util::getThreadId(), 0);
//                serialization::Data debugData = toData(request);
//                return toObject<V>(debugData);
                return invoke<V>(request, keyData);
            };

            V remove(const K& key) {
                serialization::Data keyData = toData(key);
                map::RemoveRequest request(instanceName, keyData, util::getThreadId());
                return invoke<V>(request, keyData);
            };

            bool remove(const K& key, const V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::RemoveIfSameRequest request(instanceName, keyData, valueData, util::getThreadId());
                return invoke<bool>(request, keyData);;
            };

            void deleteEntry(const K& key) {
                serialization::Data keyData = toData(key);
                map::DeleteRequest request(instanceName, keyData, util::getThreadId());
                invoke<bool>(request, keyData);;
            };

            void flush() {
                map::FlushRequest request(instanceName);
                invoke<bool>(request);
            };

            //TODO putAsync, getAsync, RemoveAsync


            bool tryRemove(const K& key, long timeoutInMillis) {
                serialization::Data keyData = toData(key);
                map::TryRemoveRequest request(instanceName, keyData, util::getThreadId(), timeoutInMillis);
                return invoke<bool>(request, keyData);;
            };

            bool tryPut(const K& key, const V& value, long timeoutInMillis) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TryPutRequest request(instanceName, keyData, valueData, util::getThreadId(), timeoutInMillis);
                return invoke<bool>(request, keyData);
            };

            V put(const K& key, const V& value, long ttl) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::PutRequest request(instanceName, keyData, valueData, util::getThreadId(), ttl);
                return invoke<V>(request, keyData);
            };

            void putTransient(const K& key, const V& value, long ttl) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::PutTransientRequest request(instanceName, keyData, valueData, util::getThreadId(), ttl);
                invoke<bool>(request, keyData);
            };

            bool replace(const K& key, const V& oldValue, const V& newValue) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(oldValue);
                serialization::Data newValueData = toData(newValue);
                map::ReplaceIfSameRequest request(instanceName, keyData, valueData, newValueData, util::getThreadId());
                return invoke<bool>(request, keyData);
            };

            V replace(const K& key, const V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::ReplaceRequest request(instanceName, keyData, valueData, util::getThreadId());
                return invoke<V>(request, keyData);
            };

            void set(const K& key, const V& value, long ttl) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::SetRequest request(instanceName, keyData, util::getThreadId(), ttl);
                invoke<bool>(request, keyData);
            };

            void lock(const K&  key) {
                serialization::Data keyData = toData(key);
                map::LockRequest request(instanceName, keyData, util::getThreadId());
                invoke<bool>(request, keyData);
            };

            void lock(const K&  key, long leaseTime) {
                serialization::Data keyData = toData(key);
                map::LockRequest request (instanceName, keyData, util::getThreadId(), leaseTime, -1);
                invoke(request, keyData);
            };

            bool isLocked(const K&  key) {
                serialization::Data keyData = toData(key);
                map::IsLockedRequest request(instanceName, keyData);
                return invoke<bool>(request, keyData);
            };

            bool tryLock(const K&  key) {
                return tryLock(key, 0);
            };

            bool tryLock(const K&  key, long timeInMillis) {
                serialization::Data keyData = toData(key);
                map::LockRequest request(instanceName, keyData, util::getThreadId(), 0, timeInMillis);

                return invoke<bool>(request, keyData);;
            };

            void unlock(const K&  key) {
                serialization::Data keyData = toData(key);
                map::UnlockRequest request(instanceName, keyData, util::getThreadId(), false);
                invoke<bool>(request, keyData);
            };

            void forceUnlock(const K&  key) {
                serialization::Data keyData = toData(key);
                map::UnlockRequest request(instanceName, keyData, util::getThreadId(), true);
                invoke<bool>(request, keyData);
            };

//        String addInterceptor(MapInterceptor interceptor) {
//                MapAddInterceptorRequest request = new MapAddInterceptorRequest(name, interceptor);
//                return invoke(request);
//            }
//
//        void removeInterceptor(String id) {
//                MapRemoveInterceptorRequest request = new MapRemoveInterceptorRequest(name, id);
//                invoke(request);
//            }

            template < typename L>
            long addEntryListener(L& listener, bool includeValue) {
                map::AddEntryListenerRequest request(instanceName, includeValue);
                impl::EntryEventHandler<K, V, L> entryEventHandler(instanceName, context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return context->getServerListenerService().template listen<map::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(instanceName, request, entryEventHandler);
            };

            template < typename L>
            long addEntryListener(L& listener, const K& key, bool includeValue) {
                serialization::Data keyData = toData(key);
                map::AddEntryListenerRequest request(instanceName, includeValue, keyData);
                impl::EntryEventHandler<K, V, L> entryEventHandler(instanceName, context->getClusterService(), context->getSerializationService(), listener, includeValue);
                return context->getServerListenerService().template listen<map::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(instanceName, request, keyData, entryEventHandler);
            };

            bool removeEntryListener(long registrationId) {
                return context->getServerListenerService().stopListening(instanceName, registrationId);
            };


            map::EntryView<K, V> getEntryView(const K& key) {
                serialization::Data keyData = toData(key);
                map::GetEntryViewRequest request(instanceName, keyData);
                map::EntryView<serialization::Data, serialization::Data> dataEntryView = invoke<map::EntryView<serialization::Data, serialization::Data>>(request, keyData);
                return map::EntryView<K, V>(toObject<K>(dataEntryView.key), toObject<V>(dataEntryView.value), dataEntryView);
            };


            bool evict(const K& key) {
                serialization::Data keyData = toData(key);
                map::EvictRequest request(instanceName, keyData, util::getThreadId());
                return invoke<bool>(request, keyData);
            };

            std::vector<K> keySet() {
                map::KeySetRequest request(instanceName);
                map::MapKeySet mapKeySet = invoke(request);
                return mapKeySet.getKeySet();
            };

            std::map< K, V > getAll(const std::set<K>& keys) {
                std::vector<serialization::Data> keySet(keys.size());
                for (typename std::set<K>::iterator it = keys.begin(); it != keys.end(); ++it) {
                    keySet.push_back(toData(*it));
                }
                map::GetAllRequest request(instanceName, keySet);
                map::MapEntrySet mapEntrySet = invoke < map::MapEntrySet >(request);
                std::map< K, V > result;
                const std::vector< std::pair< serialization::Data, serialization::Data> >  & entrySet = mapEntrySet.getEntrySet();
                for (std::vector< std::pair< serialization::Data, serialization::Data> >::const_iterator it = entrySet.begin(); it != entrySet.end(); ++it) {
                    result[toObject<K>(it->first)] = toObject<V>(it->second);
                }
                return result;
            };

            std::vector<V> values() {
                map::ValuesRequest request(instanceName);
                map::MapValueCollection valueCollection = invoke < map::MapValueCollection >(request);
                const vector<serialization::Data>  & getValues = valueCollection.getValues();
                std::vector<V> values(getValues.size());
                for (int i = 0; i < getValues.size(); i++) {
                    values[i] = toObject<V>(getValues[i]);
                }
                return values;
            };

            std::vector< std::pair<K, V> > entrySet() {
                map::EntrySetRequest request(instanceName);
                map::MapEntrySet result = invoke < map::MapEntrySet >(request);
                const std::vector< std::pair< serialization::Data, serialization::Data> >  & returnedEntries = result.getEntrySet();
                std::vector< std::pair<K, V> > entrySet(returnedEntries.size());
                for (int i = 0; i < entrySet.size(); ++i) {
                    entrySet[i] = std::make_pair<K, V>(toObject<K>(returnedEntries[i].first), toObject<V>(returnedEntries[i].second));
                }
                return entrySet;
            };

            std::vector<K> keySet(const std::string& sql) {
                map::QueryRequest request(instanceName, "KEY", sql);
                map::QueryDataResultStream queryDataResultStream = invoke(request);
                const vector<map::QueryResultEntry>  & dataResult = queryDataResultStream.getResultData();
                std::vector<K> keySet(dataResult.size());
                for (int i = 0; i <dataResult.size(); ++i) {
                    keySet[i] = toObject<K>(dataResult[i].key);
                }
                return keySet;
            };

            std::vector<V> values(const std::string& sql) {
                map::QueryRequest request(instanceName, "VALUE", sql);
                map::QueryDataResultStream queryDataResultStream = invoke(request);
                const vector<map::QueryResultEntry>  & dataResult = queryDataResultStream.getResultData();
                std::vector<V> keySet(dataResult.size());
                for (int i = 0; i < dataResult.size(); ++i) {
                    keySet[i] = toObject<V>(dataResult[i].value);
                }
                return keySet;
            };

            std::vector<std::pair<K, V> > entrySet(const std::string& sql) {
                map::QueryRequest request(instanceName, "ENTRY", sql);
                map::QueryDataResultStream queryDataResultStream = invoke(request);
                const vector<map::QueryResultEntry>  & dataResult = queryDataResultStream.getResultData();
                std::vector<std::pair<K, V> > keySet(dataResult.size());
                for (int i = 0; i < dataResult.size(); ++i) {
                    keySet[i] = std::make_pair(toObject<K>(dataResult[i].key), toObject<V>(dataResult[i].value));
                }
                return keySet;
            };

            void addIndex(const string& attribute, bool ordered) {
                map::AddIndexRequest request(instanceName, attribute, ordered);
                invoke<bool>(request);
            };

//        Object executeOnKey(K key, EntryProcessor entryProcessor) { TODO
//                final Data keyData = toData(key);
//                MapExecuteOnKeyRequest request = new MapExecuteOnKeyRequest(name, entryProcessor, keyData);
//                return invoke(request, keyData);
//            }
//
//        Map<K, Object> executeOnEntries(EntryProcessor entryProcessor) {
//                MapExecuteOnAllKeysRequest request = new MapExecuteOnAllKeysRequest(name, entryProcessor);
//                MapEntrySet entrySet = invoke(request);
//                Map<K, Object> result = new HashMap<K, Object>();
//                for (Entry<Data, Data> dataEntry : entrySet.getEntrySet()) {
//                    final Data keyData = dataEntry.getKey();
//                    final Data valueData = dataEntry.getValue();
//                    K key = toObject(keyData);
//                    result.put(key, toObject(valueData));
//                }
//                return result;
//            }

            void set(K key, V value) {
                set(key, value, -1);
            };

            int size() {
                map::SizeRequest request(instanceName);
                return invoke<int>(request);;
            };

            bool isEmpty() {
                return size() == 0;
            };

            void putAll(const std::map<K, V>& m) {
                map::MapEntrySet entrySet;
                std::vector< std::pair< serialization::Data, serialization::Data> >  & entryDataSet = entrySet.getEntrySet();
                entryDataSet.resize(m.size());
                int i = 0;
                for (typename std::map<K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
                    entryDataSet[i] = std::make_pair(toData(it->first), toData(it->second));
                }
                map::PutAllRequest request(instanceName, entrySet);
                invoke<bool>(request);
            };

            void clear() {
                map::ClearRequest request(instanceName);
                invoke<bool>(request);
            };

        private:
            void onDestroy() {
                map::DestroyRequest request (instanceName);
                invoke<bool>(request);
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

            std::string instanceName;
            spi::ClientContext *context;
        };
    }
}

#endif /* HAZELCAST_IMAP */