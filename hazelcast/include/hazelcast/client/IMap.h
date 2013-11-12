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
#include "map/ClearRequest.h"
#include "map/PutAllRequest.h"
#include "map/QueryRequest.h"
#include "map/EntryView.h"
#include "map/AddEntryListenerRequest.h"
#include "map/ExecuteOnKeyRequest.h"
#include "map/ExecuteOnAllKeysRequest.h"
#include "map/AddInterceptorRequest.h"
#include "map/RemoveInterceptorRequest.h"
#include "map/PutIfAbsentRequest.h"
#include "impl/EntryListener.h"
#include "impl/EntryEventHandler.h"
#include "impl/PortableEntryEvent.h"
#include "impl/QueryResultSet.h"
#include "serialization/SerializationService.h"
#include "Future.h"
#include "proxy/DistributedObject.h"
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdexcept>

namespace hazelcast {
    namespace client {

        template<typename K, typename V>
        class IMap : public proxy::DistributedObject {
            friend class HazelcastClient;

        public:

            bool containsKey(const K &key) {
                serialization::Data keyData = toData(key);
                map::ContainsKeyRequest request(getName(), keyData);
                return invoke<bool>(request, keyData);
            };

            bool containsValue(const V &value) {
                serialization::Data valueData = toData(value);
                map::ContainsValueRequest request(getName(), valueData);
                return invoke<bool>(request, valueData);
            };

            V get(const K &key) {
                serialization::Data keyData = toData(key);
                map::GetRequest request(getName(), keyData);
                return invoke<V>(request, keyData);
            };

            V put(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::PutRequest request(getName(), keyData, valueData, util::getThreadId(), 0);
                return invoke<V>(request, keyData);
            };

            V remove(const K &key) {
                serialization::Data keyData = toData(key);
                map::RemoveRequest request(getName(), keyData, util::getThreadId());
                return invoke<V>(request, keyData);
            };

            bool remove(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::RemoveIfSameRequest request(getName(), keyData, valueData, util::getThreadId());
                return invoke<bool>(request, keyData);;
            };

            void deleteEntry(const K &key) {
                serialization::Data keyData = toData(key);
                map::DeleteRequest request(getName(), keyData, util::getThreadId());
                invoke<bool>(request, keyData);;
            };

            void flush() {
                map::FlushRequest request(getName());
                invoke<bool>(request);
            };

            Future<V> putAsync(const K &key, const V &value, long ttlInMillis) {
                Future<V> future;
                boost::thread t(boost::bind(asyncPutThread, boost::ref(*this), key, value, ttlInMillis, future));
                return future;
            };

            Future<V> putAsync(const K &key, const V &value) {
                Future<V> future;
                boost::thread t(boost::bind(asyncPutThread, boost::ref(*this), key, value, -1, future));
                return future;
            };

            Future<V> getAsync(const K &key) {
                Future<V> future;
                boost::thread t(boost::bind(asyncGetThread, boost::ref(*this), key, future));
                return future;
            };

            Future<V> removeAsync(const K &key) {
                Future<V> future;
                boost::thread t(boost::bind(asyncRemoveThread, boost::ref(*this), key, future));
                return future;
            };

            bool tryRemove(const K &key, long timeoutInMillis) {
                serialization::Data keyData = toData(key);
                map::TryRemoveRequest request(getName(), keyData, util::getThreadId(), timeoutInMillis);
                return invoke<bool>(request, keyData);;
            };

            bool tryPut(const K &key, const V &value, long timeoutInMillis) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::TryPutRequest request(getName(), keyData, valueData, util::getThreadId(), timeoutInMillis);
                return invoke<bool>(request, keyData);
            };

            V put(const K &key, const V &value, long ttlInMillis) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::PutRequest request(getName(), keyData, valueData, util::getThreadId(), ttlInMillis);
                return invoke<V>(request, keyData);
            };

            void putTransient(const K &key, const V &value, long ttlInMillis) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::PutTransientRequest request(getName(), keyData, valueData, util::getThreadId(), ttlInMillis);
                invoke<bool>(request, keyData);
            };

            V putIfAbsent(const K &key, const V &value) {
                return putIfAbsent(key, value, -1);
            }

            V putIfAbsent(const K &key, const V &value, long ttlInMillis) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::PutIfAbsentRequest request(getName(), keyData, valueData, util::getThreadId(), ttlInMillis);
                return invoke<V>(request, keyData);
            }

            bool replace(const K &key, const V &oldValue, const V &newValue) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(oldValue);
                serialization::Data newValueData = toData(newValue);
                map::ReplaceIfSameRequest request(getName(), keyData, valueData, newValueData, util::getThreadId());
                return invoke<bool>(request, keyData);
            };

            V replace(const K &key, const V &value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::ReplaceRequest request(getName(), keyData, valueData, util::getThreadId());
                return invoke<V>(request, keyData);
            };

            void set(const K &key, const V &value, long ttl) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(value);
                map::SetRequest request(getName(), keyData, valueData, util::getThreadId(), ttl);
                invoke<bool>(request, keyData);
            };

            void lock(const K &key) {
                serialization::Data keyData = toData(key);
                map::LockRequest request(getName(), keyData, util::getThreadId());
                invoke<bool>(request, keyData);
            };

            void lock(const K &key, long leaseTime) {
                serialization::Data keyData = toData(key);
                map::LockRequest request (getName(), keyData, util::getThreadId(), leaseTime, -1);
                invoke<bool>(request, keyData);
            };

            bool isLocked(const K &key) {
                serialization::Data keyData = toData(key);
                map::IsLockedRequest request(getName(), keyData);
                return invoke<bool>(request, keyData);
            };

            bool tryLock(const K &key) {
                return tryLock(key, 0);
            };

            bool tryLock(const K &key, long timeInMillis) {
                serialization::Data keyData = toData(key);
                map::LockRequest request(getName(), keyData, util::getThreadId(), LONG_MAX, timeInMillis);

                return invoke<bool>(request, keyData);;
            };

            void unlock(const K &key) {
                serialization::Data keyData = toData(key);
                map::UnlockRequest request(getName(), keyData, util::getThreadId(), false);
                invoke<bool>(request, keyData);
            };

            void forceUnlock(const K &key) {
                serialization::Data keyData = toData(key);
                map::UnlockRequest request(getName(), keyData, util::getThreadId(), true);
                invoke<bool>(request, keyData);
            };

            template<typename MapInterceptor>
            std::string addInterceptor(const MapInterceptor &interceptor) {
                map::AddInterceptorRequest<MapInterceptor> request(getName(), interceptor);
                return invoke<std::string>(request);
            }

            void removeInterceptor(const std::string &id) {
                map::RemoveInterceptorRequest request(getName(), id);
                invoke<bool>(request);
            }

            template < typename L>
            long addEntryListener(L &listener, bool includeValue) {
                map::AddEntryListenerRequest request(getName(), includeValue);
                impl::EntryEventHandler<K, V, L> entryEventHandler(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener, includeValue);
                return getContext().getServerListenerService().template listen<map::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(request, entryEventHandler);
            };

            template < typename L>
            long addEntryListener(L &listener, const K &key, bool includeValue) {
                serialization::Data keyData = toData(key);
                serialization::Data cloneData = keyData.clone();
                map::AddEntryListenerRequest request(getName(), includeValue, keyData);
                impl::EntryEventHandler<K, V, L> entryEventHandler(getName(), getContext().getClusterService(), getContext().getSerializationService(), listener, includeValue);
                return getContext().getServerListenerService().template listen<map::AddEntryListenerRequest, impl::EntryEventHandler<K, V, L>, impl::PortableEntryEvent >(request, cloneData, entryEventHandler);
            };

            bool removeEntryListener(long registrationId) {
                return getContext().getServerListenerService().stopListening(registrationId);
            };


            map::EntryView<K, V> getEntryView(const K &key) {
                serialization::Data keyData = toData(key);
                map::GetEntryViewRequest request(getName(), keyData);
                map::EntryView<serialization::Data, serialization::Data> dataEntryView = invoke<map::EntryView<serialization::Data, serialization::Data> >(request, keyData);
                return map::EntryView<K, V>(toObject<K>(dataEntryView.key), toObject<V>(dataEntryView.value), dataEntryView);
            };


            bool evict(const K &key) {
                serialization::Data keyData = toData(key);
                map::EvictRequest request(getName(), keyData, util::getThreadId());
                return invoke<bool>(request, keyData);
            };

            std::vector<K> keySet() {
                map::KeySetRequest request(getName());
                map::MapKeySet dataKeySet = invoke<map::MapKeySet>(request);
                std::vector<serialization::Data> const &dataResult = dataKeySet.getKeySet();
                std::vector<K> keySet(dataResult.size());
                for (int i = 0; i < dataResult.size(); ++i) {
                    keySet[i] = toObject<K>(dataResult[i]);
                }
                return keySet;
            };

            std::map< K, V > getAll(const std::set<K> &keys) {
                std::vector<serialization::Data> keySet(keys.size());
                int i = 0;
                for (typename std::set<K>::iterator it = keys.begin(); it != keys.end(); ++it) {
                    keySet[i++] = toData(*it);
                }
                map::GetAllRequest request(getName(), keySet);
                map::MapEntrySet mapEntrySet = invoke < map::MapEntrySet >(request);
                std::map< K, V > result;
                const std::vector< std::pair< serialization::Data, serialization::Data> > &entrySet = mapEntrySet.getEntrySet();
                for (std::vector< std::pair< serialization::Data, serialization::Data> >::const_iterator it = entrySet.begin(); it != entrySet.end(); ++it) {
                    result[toObject<K>(it->first)] = toObject<V>(it->second);
                }
                return result;
            };

            std::vector<V> values() {
                map::ValuesRequest request(getName());
                map::MapValueCollection valueCollection = invoke < map::MapValueCollection >(request);
                const vector<serialization::Data> &getValues = valueCollection.getValues();
                std::vector<V> values(getValues.size());
                for (int i = 0; i < getValues.size(); i++) {
                    values[i] = toObject<V>(getValues[i]);
                }
                return values;
            };

            std::vector< std::pair<K, V> > entrySet() {
                map::EntrySetRequest request(getName());
                map::MapEntrySet result = invoke < map::MapEntrySet >(request);
                const std::vector< std::pair< serialization::Data, serialization::Data> > &returnedEntries = result.getEntrySet();
                std::vector< std::pair<K, V> > entrySet(returnedEntries.size());
                for (int i = 0; i < entrySet.size(); ++i) {
                    entrySet[i] = std::make_pair<K, V>(toObject<K>(returnedEntries[i].first), toObject<V>(returnedEntries[i].second));
                }
                return entrySet;
            };

            std::vector<K> keySet(const std::string &sql) {
                map::QueryRequest request(getName(), "KEY", sql);
                impl::QueryResultSet queryDataResultStream = invoke<impl::QueryResultSet>(request);
                const vector<impl::QueryResultEntry> &dataResult = queryDataResultStream.getResultData();
                std::vector<K> keySet(dataResult.size());
                for (int i = 0; i < dataResult.size(); ++i) {
                    keySet[i] = toObject<K>(dataResult[i].key);
                }
                return keySet;
            };

            std::vector<V> values(const std::string &sql) {
                map::QueryRequest request(getName(), "VALUE", sql);
                impl::QueryResultSet queryDataResultStream = invoke<impl::QueryResultSet>(request);
                const vector<impl::QueryResultEntry> &dataResult = queryDataResultStream.getResultData();
                std::vector<V> keySet(dataResult.size());
                for (int i = 0; i < dataResult.size(); ++i) {
                    keySet[i] = toObject<V>(dataResult[i].value);
                }
                return keySet;
            };

            std::vector<std::pair<K, V> > entrySet(const std::string &sql) {
                map::QueryRequest request(getName(), "ENTRY", sql);
                impl::QueryResultSet queryDataResultStream = invoke<impl::QueryResultSet>(request);
                const vector<impl::QueryResultEntry> &dataResult = queryDataResultStream.getResultData();
                std::vector<std::pair<K, V> > keySet(dataResult.size());
                for (int i = 0; i < dataResult.size(); ++i) {
                    keySet[i] = std::make_pair(toObject<K>(dataResult[i].key), toObject<V>(dataResult[i].value));
                }
                return keySet;
            };

            void addIndex(const string &attribute, bool ordered) {
                map::AddIndexRequest request(getName(), attribute, ordered);
                invoke<bool>(request);
            };

            template<typename ResultType, typename EntryProcessor>
            ResultType executeOnKey(const K &key, EntryProcessor &entryProcessor) {
                serialization::Data keyData = toData(key);
                map::ExecuteOnKeyRequest<EntryProcessor> request(getName(), entryProcessor, keyData);
                return invoke<ResultType>(request, keyData);
            }

            template<typename ResultType, typename EntryProcessor>
            std::map<K, ResultType> executeOnEntries(EntryProcessor &entryProcessor) {
                map::ExecuteOnAllKeysRequest<EntryProcessor> request(getName(), entryProcessor);
                map::MapEntrySet mapEntrySet = invoke< map::MapEntrySet>(request);
                std::map<K, ResultType> result;
                const std::vector< std::pair< serialization::Data, serialization::Data> > &entrySet = mapEntrySet.getEntrySet();
                for (std::vector< std::pair< serialization::Data, serialization::Data> >::const_iterator it = entrySet.begin(); it != entrySet.end(); ++it) {
                    K key = toObject<K>(it->first);
                    ResultType resultType = toObject<ResultType>(it->second);
                    result[key] = resultType;
                }
                return result;
            }

            void set(K key, V value) {
                set(key, value, -1);
            };

            int size() {
                map::SizeRequest request(getName());
                return invoke<int>(request);
            };

            bool isEmpty() {
                return size() == 0;
            };

            void putAll(const std::map<K, V> &m) {
                map::MapEntrySet entrySet;
                std::vector< std::pair< serialization::Data, serialization::Data> > &entryDataSet = entrySet.getEntrySet();
                entryDataSet.resize(m.size());
                int i = 0;
                for (typename std::map<K, V>::const_iterator it = m.begin(); it != m.end(); ++it) {
                    entryDataSet[i++] = std::make_pair(toData(it->first), toData(it->second));
                }
                map::PutAllRequest request(getName(), entrySet);
                invoke<bool>(request);
            };

            void clear() {
                map::ClearRequest request(getName());
                invoke<bool>(request);
            };

            /**
             * Destroys this object cluster-wide.
             * Clears and releases all resources for this object.
             */
            void onDestroy() {
            };

        private:
            IMap(const std::string &instanceName, spi::ClientContext *context)
            : DistributedObject("hz:impl:mapService", instanceName, context) {

            }

            template<typename T>
            serialization::Data toData(const T &object) {
                return getContext().getSerializationService().template toData<T>(&object);
            };

            template<typename T>
            T toObject(const serialization::Data &data) {
                return getContext().getSerializationService().template toObject<T>(data);
            };

            template<typename Response, typename Request>
            Response invoke(const Request &request, serialization::Data &keyData) {
                return getContext().getInvocationService().template invokeOnKeyOwner<Response>(request, keyData);
            };

            template<typename Response, typename Request>
            Response invoke(const Request &request) {
                return getContext().getInvocationService().template invokeOnRandomTarget<Response>(request);
            };

            static void asyncPutThread(IMap &map, const K key, const V value, long ttlInMillis, Future<V> future) {
                V *v = NULL;
                try{
                    v = new V(map.put(key, value, ttlInMillis));
                    future.accessInternal().setValue(v);
                } catch(std::exception &ex){
                    future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
                } catch(...){
                    std::cerr << "Exception in IMap::asyncPutThread" << std::endl;
                }
            }

            static void asyncRemoveThread(IMap &map, const K key, Future<V> future) {
                V *v = NULL;
                try{
                    v = new V(map.remove(key));
                    future.accessInternal().setValue(v);
                } catch(std::exception &ex){
                    future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
                } catch(...){
                    std::cerr << "Exception in IMap::asyncRemoveThread" << std::endl;
                }
            }

            static void asyncGetThread(IMap &map, const K key, Future<V> future) {
                V *v = NULL;
                try{
                    v = new V(map.get(key));
                    future.accessInternal().setValue(v);
                } catch(std::exception &ex){
                    future.accessInternal().setException(new exception::IException("ServerNode", ex.what()));
                } catch(...){
                    std::cerr << "Exception in IMap::asyncGetThread" << std::endl;
                }
            }

        };
    }
}

#endif /* HAZELCAST_IMAP */