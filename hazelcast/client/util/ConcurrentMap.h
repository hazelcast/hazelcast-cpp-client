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

                /**
                *
                * @return the previous value associated with the specified key,
                *         or <tt>null</tt> if there was no mapping for the key
                * @throws NullPointerException if the specified key or value is null
                */
                V *putIfAbsent(const K& key, V& value) {
                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
                    if (internalMap.count(&key) > 0) {
                        return internalMap[&key];
                    } else {
                        internalMap[&key] = &value;
                        return NULL;
                    }
                };

//                void put(const K& key, V& value) {
//                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
//                    value = internalMap[key];
//                };

                /**
                 * Returns the value to which the specified key is mapped,
                 * or {@code null} if this map contains no mapping for the key.
                 *
                 */
                V *get(const K& key) {
                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
                    if (internalMap.count(&key) > 0)
                        return internalMap[&key];
                    else
                        return NULL;
                };

//                V& operator [](const K& key) {
//                    boost::lock_guard<boost::unique_lock<boost::mutex > > lock_guard(mapLock);
//                    return internalMap[key];
//                };


            private:
                boost::unique_lock<boost::mutex> mapLock;
                std::map<K const *, V *> internalMap;
            };
        }
    }
}

#endif //HAZELCAST_CONCURRENT_MAP
