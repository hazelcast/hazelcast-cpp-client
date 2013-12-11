//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_MAP
#define HAZELCAST_CONCURRENT_MAP

#include "hazelcast/util/HazelcastDll.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <map>
#include <vector>

namespace hazelcast {
    namespace util {
        template <typename K, typename V, typename Comparator  = std::less<K> >
        /**
        * Put once, read multiple thread safe map
        * Deletes nothing.
        * the ptr getted by any operation should be deleted
        * only if it is certain that they will never be used again.
        *
        */
        class HAZELCAST_API ConcurrentMap {
        public:
            ConcurrentMap() {

            };

            ~ConcurrentMap() {
				/*boost::lock_guard<boost::mutex> lg(mapLock);
                std::vector<V *> valueArray(internalMap.size());
                typename std::map<K, V *, Comparator>::iterator it;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    delete = it->second;
                }
                return valueArray;*/
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
            V *putIfAbsent(const K& key, V *value) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0) {
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return NULL;
                }
            };

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            V *put(const K& key, V *value) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                V *returnValue = NULL;
                if (internalMap.count(key) > 0) {
                    returnValue = internalMap[key];
                }
                internalMap[key] = value;
                return returnValue;
            };

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code null} if this map contains no mapping for the key.
             *
             */
            V *get(const K& key) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0)
                    return internalMap[key];
                else
                    return NULL;
            };

            std::vector<V *> values() {
                boost::lock_guard<boost::mutex> lg(mapLock);
                std::vector<V *> valueArray(internalMap.size());
                typename std::map<K, V *, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    valueArray[i++] = it->second;
                }
                return valueArray;
            }

        private:
            std::map<K, V *, Comparator> internalMap;
            mutable boost::mutex mapLock;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_MAP
