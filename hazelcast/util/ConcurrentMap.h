//
// Created by sancar koyunlu on 5/21/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.



#ifndef HAZELCAST_CONCURRENT_MAP
#define HAZELCAST_CONCURRENT_MAP

#include <map>

namespace hazelcast {
    namespace util {
        template <typename K, typename V>
        class ConcurrentMap {
        public:
            ConcurrentMap() {

            };

            ~ConcurrentMap() {
                typename std::map<K, V *>::iterator it;
                for (it = internalMap.begin(); it != internalMap.end(); it++) {
//                    delete it->second; //TODO uncomment on release
                }
            };


            bool containsKey(const K& key) const {
                return internalMap.count(key) > 0;
            };

            /**
            *
            * @return the previous value associated with the specified key,
            *         or <tt>null</tt> if there was no mapping for the key
            * @throws NullPointerException if the specified key or value is null
            */
            V *putIfAbsent(const K& key, V *value) {
                if (internalMap.count(key) > 0) {
                    delete value;
                    return internalMap[key];
                } else {
                    internalMap[key] = value;
                    return NULL;
                }
            };

//                void put(const K& key, V& value) {
//                    value = internalMap[key];
//                };

            /**
             * Returns the value to which the specified key is mapped,
             * or {@code null} if this map contains no mapping for the key.
             *
             */
            V *get(const K& key) {
                if (internalMap.count(key) > 0)
                    return internalMap[key];
                else
                    return NULL;
            };

//                V& operator [](const K& key) {
//                    return internalMap[key];
//                };


        private:
            std::map<K, V *> internalMap;
        };
    }
}

#endif //HAZELCAST_CONCURRENT_MAP
