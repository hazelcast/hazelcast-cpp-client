#ifndef HAZELCAST_IMAP
#define HAZELCAST_IMAP


#include "HazelcastException.h"
#include "serialization/Data.h"
#include "serialization/SerializationService.h"
#include "spi/ClientContext.h"
#include "spi/InvocationService.h"
#include "map/GetRequest.h"
#include "map/PutRequest.h"
#include "map/RemoveRequest.h"
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

            IMap(std::string instanceName, spi::ClientContext& clientContext)
            : instanceName(instanceName)
            , context(clientContext) {

            };

            IMap(const IMap& rhs)
            : instanceName(rhs.instanceName)
            , context(rhs.context) {
            };

            std::string getName() const {
                return instanceName;
            };

            bool containsKey(const K& key) {
            };

            bool containsValue(const V& value) {
            };

            V *get(const K& key) {
                serialization::Data keyData = toData(key);
                map::GetRequest request(instanceName, keyData);
                V *value = new V();
                invoke(request, *value, keyData);
                return value;
            };

            V *put(const K& key, V& value) {
                serialization::Data keyData = toData(key);
                serialization::Data valueData = toData(key);
                map::PutRequest request(instanceName, keyData, valueData, 1, 0);
                V *oldValue = new V();
                invoke(request, value, keyData);
                return oldValue;
            };

            V *remove(const K& key) {
                serialization::Data keyData = toData(key);
                map::RemoveRequest request(instanceName, keyData, 1);
                V *value = new V();
                invoke(request, *value, keyData);
                return value;
            };

            void flush() {
            };

            std::map< K, V > getAll(const std::set<K>& keys) {
            };

            bool tryRemove(const K& key, long timeoutInMillis) {
            };

            bool tryPut(const K& key, const V& value, long timeoutInMillis) {
            };

            void put(const K& key, const V& value, long ttl) {
            };

            void putTransient(const K& key, const V& value, long ttl) {
            };

            bool replace(const K& key, V oldValue, V newValue) {
            };

            //TODO V replace(K,V)
            //TODO set
            //TODO locks and listeners

            std::pair<K, V> getEntry(const K& key) {
            };

            //predicates ? supported TODO

            bool evict(const K& key) {
            };

            std::set<K> keySet() {
            };

            std::vector<V> values() {
            };

            std::vector< std::pair<K, V> > entrySet() {
            };

            void lock(const K& key) throw (hazelcast::client::HazelcastException) {
            };

            bool isLocked(const K& key) {
            };

            bool tryLock(const K& key, long timeoutInMillis) {
            };

            void unlock(const K& key) {
            };

            void forceUnlock(const K& key) {
            };

        private:
            template<typename T>
            hazelcast::client::serialization::Data toData(const T& object) {
                return context.getSerializationService().toData(object);
            };

//
            template<typename T>
            T toObject(const hazelcast::client::serialization::Data& data) {
                return context.getSerializationService().toObject(data);
            };

//
            template<typename Request, typename Response>
            void invoke(const Request& request, Response& response, const hazelcast::client::serialization::Data&  keyData) {
//                try {
//                context.getInvocationService().invokeOnKeyOwner(request, response, keyData); //TODO real one
                context.getInvocationService().invokeOnRandomTarget(request, response); //TODO delete line later
//                } catch (Exception e) {
//                    throw ExceptionUtil.rethrow(e);
//                }
            };

//
            template<typename Request, typename Response>
            void invoke(const Request& request, Response& response) {
//                try {
                context.getInvocationService().invokeOnRandomTarget(request, response);
//                } catch (Exception e) {
//                    throw ExceptionUtil.rethrow(e);
//                }
            };

            std::string instanceName;
            spi::ClientContext& context;
        };
    }
}

#endif /* HAZELCAST_IMAP */