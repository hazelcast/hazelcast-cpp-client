//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_SYNCHRONIZED_MAP
#define HAZELCAST_SYNCHRONIZED_MAP

#include "hazelcast/util/HazelcastDll.h"
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>
#include <map>
#include <vector>

namespace hazelcast {
    namespace util {
        template <typename K, typename V, typename Comparator  = std::less<K> >
        class HAZELCAST_API SynchronizedMap {
        public:
            SynchronizedMap() {

            };

            ~SynchronizedMap() {
                boost::lock_guard<boost::mutex> lg(mapLock);
                internalMap.clear();
            };

            bool containsKey(const K &key) const {
                boost::lock_guard<boost::mutex> guard (mapLock);
                return internalMap.count(key) > 0;
            };

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            boost::shared_ptr<V> putIfAbsent(const K &key, boost::shared_ptr<V> value) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0) {
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return boost::shared_ptr<V>();
                }
            };

            /**
             *
             * @return the previous value associated with the specified key,
             *         or <tt>null</tt> if there was no mapping for the key
             */
            boost::shared_ptr<V> put(const K &key, boost::shared_ptr<V> value) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                boost::shared_ptr<V> returnValue;
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
            boost::shared_ptr<V> get(const K &key) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0)
                    return internalMap[key];
                else
                    return boost::shared_ptr<V>();
            };

            /**
            * Returns the value to which the specified key is mapped,
            * and removes from map
            * or {@code null} if this map contains no mapping for the key.
            *
            */
            boost::shared_ptr<V> remove(const K &key) {
                boost::lock_guard<boost::mutex> lg(mapLock);
                if (internalMap.count(key) > 0) {
                    boost::shared_ptr<V> v = internalMap[key];
                    internalMap.erase(key);
                    return v;
                }
                else
                    return boost::shared_ptr<V>();
            };

            std::vector<std::pair<K, boost::shared_ptr<V> > > entrySet() {
                boost::lock_guard<boost::mutex> lg(mapLock);
                std::vector<std::pair<K, boost::shared_ptr<V> > > entries(internalMap.size());
                typename std::map<K, boost::shared_ptr<V>, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    entries[i++] = std::pair<K, boost::shared_ptr<V> >(it->first, it->second);
                }
                return entries;
            }

            std::vector<std::pair<K, boost::shared_ptr<V> > > clear() {
                boost::lock_guard<boost::mutex> lg(mapLock);
                std::vector<std::pair<K, boost::shared_ptr<V> > > entries(internalMap.size());
                typename std::map<K, boost::shared_ptr<V>, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    entries[i++] = std::pair<K, boost::shared_ptr<V> >(it->first, it->second);
                }
                internalMap.clear();
                return entries;
            }

            std::vector<boost::shared_ptr<V> > values() {
                boost::lock_guard<boost::mutex> lg(mapLock);
                std::vector<boost::shared_ptr<V> > valueArray(internalMap.size());
                typename std::map<K, boost::shared_ptr<V>, Comparator>::iterator it;
                int i = 0;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
                    valueArray[i++] = it->second;
                }
                return valueArray;
            }

        private:
            std::map<K, boost::shared_ptr<V>, Comparator> internalMap;
            mutable boost::mutex mapLock;
        };
    }
}

#endif //HAZELCAST_SYNCHRONIZED_MAP
