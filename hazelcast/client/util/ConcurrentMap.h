//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_MAP
#define HAZELCAST_CONCURRENT_MAP

#include <map>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/mutex.hpp>

namespace hazelcast {
    namespace client {
        namespace util {
            template <typename K, typename V>
            class ConcurrentMap {
            public:
                ConcurrentMap() {

                };

                /*
                    return true if put is made
                    returns false if there is already a value , value updated
                 */
                bool putIfAbsent(const K& key, V& value) {
                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
                    if (internalMap.count(key) > 0) {
                        internalMap[key] = value;
                        return true;
                    } else {
                        value = internalMap[key];
                        return false;
                    }
                };

                void put(const K& key, V& value) {
                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
                    value = internalMap[key];
                };

                V& get(const K& key) {
                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
                    return internalMap[key];
                };

                V& operator [](const K& key) {
                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
                    return internalMap[key];
                };


            private:
                boost::unique_lock<boost::mutex> mapLock;
                std::map<K, V> internalMap;
            };
        }
    }
}

#endif //HAZELCAST_CONCURRENT_MAP
