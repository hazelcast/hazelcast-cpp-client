//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_MAP
#define HAZELCAST_CONCURRENT_MAP

#include "hazelcast/util/Lock.h"
#include "hazelcast/util/LockGuard.h"
#include <map>
#include <vector>

namespace hazelcast {
    namespace util {
        template <typename K, typename V>
        class ConcurrentMap {
        public:
            ConcurrentMap() {

            };

            ~ConcurrentMap() {
            };

            bool containsKey(const K& key) const {
                LockGuard lg(lock);
                return internalMap.count(key) > 0;
            };

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            V *putIfAbsent(const K& key, V *value) {
                LockGuard lg(lock);
                if (internalMap.count(key) > 0) {
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return NULL;
                }
            };

            V *put(const K& key, V *value) {
                LockGuard lg(lock);
                if (internalMap.count(key) > 0) {
                    V *tempValue = internalMap[key];
                    internalMap[key] = value;
                    return tempValue;
                } else {
                    internalMap[key] = value;
                    return NULL;
                }
            };

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code null} if this map contains no mapping for the key.
             *
             */
            V *get(const K& key) {
                LockGuard lg(lock);
                if (internalMap.count(key) > 0)
                    return internalMap[key];
                else
                    return NULL;
            };

            V *remove(const K& key) {
                LockGuard lg(lock);
                if (internalMap.count(key) > 0) {
                    V *value = internalMap[key];
                    internalMap.erase(internalMap.find(key));
                    return value;
                }
                else
                    return NULL;
            };

            std::vector<V *> values() {
                LockGuard lg(lock);
                std::vector<V *> val(internalMap.size());
                int i = 0;
                for (typename std::map<K, V *>::iterator it = internalMap.begin(); it != internalMap.end(); ++it) {
                    val[i++] = it->second;
                }
                return val;
            };

            std::vector<K> keys() {
                LockGuard lg(lock);
                std::vector<K> k(internalMap.size());
                int i = 0;
                for (typename std::map<K, V *>::iterator it = internalMap.begin(); it != internalMap.end(); ++it) {
                    k[i++] = it->first;
                }
                return k;
            };

            void clear() {
                LockGuard lg(lock);
                for (typename std::map<K, V *>::iterator it = internalMap.begin(); it != internalMap.end(); ++it) {
                    delete it->second;
                }
                internalMap.clear();
            };

        private:
            std::map<K, V *> internalMap;
            mutable Lock lock;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_MAP
