//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_SMART_MAP
#define HAZELCAST_CONCURRENT_SMART_MAP

#include "AtomicPointer.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <map>
#include <vector>

namespace hazelcast {
    namespace util {



        template <typename K, typename V, typename Comparator  = std::less<K>, typename Hasher >
        class ConcurrentSmartMap {
        public:
            ConcurrentSmartMap() {

            };

            ~ConcurrentSmartMap() {
            };

            bool containsKey(const K& key) const {
                boost::lock_guard<boost::mutex> guard (mapLock);
                return internalMap.count(key) > 0;
            };

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            AtomicPointer<V , Hasher> putIfAbsent(const K& key, AtomicPointer<V ,Hasher> value) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0) {
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return NULL;
                }
            };

            AtomicPointer<V, Hasher> put(const K& key, V *value) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0) {
                    AtomicPointer<V, Hasher> tempValue = internalMap[key];
                    internalMap[key] = AtomicPointer<V, Hasher>(value);
                    return tempValue;
                } else {
                    internalMap[key] = AtomicPointer<V, Hasher>(value);
                    return NULL;
                }
            };

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code null} if this map contains no mapping for the key.
             *
             */
            AtomicPointer<V, Hasher> get(const K& key) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0)
                    return internalMap[key];
                else
                    return NULL;
            };

            AtomicPointer<V , Hasher> remove(const K& key) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0) {
                    AtomicPointer<V, Hasher> value = internalMap[key];
                    internalMap.erase(internalMap.find(key));
                    return value;
                }
                else
                    return NULL;
            };

            std::vector<AtomicPointer<V, Hasher> > values() {
                boost::lock_guard<boost::mutex> lg(mapLock);
                std::vector<AtomicPointer<V, Hasher> > val(internalMap.size());
                int i = 0;
                for (typename std::map<K, AtomicPointer<V, Hasher> >::iterator it = internalMap.begin(); it != internalMap.end(); ++it) {
                    val[i++] = it->second;
                }
                return val;
            };

            std::vector<K> keys() {
                boost::lock_guard<boost::mutex> lg(mapLock);
                std::vector<K> k(internalMap.size());
                int i = 0;
                for (typename std::map<K, V *>::iterator it = internalMap.begin(); it != internalMap.end(); ++it) {
                    k[i++] = it->first;
                }
                return k;
            };

        private:
            std::map<K, AtomicPointer<V, Hasher>, Comparator> internalMap;
            mutable boost::mutex mapLock;

        };

    }
}

#endif //HAZELCAST_CONCURRENT_MAP
