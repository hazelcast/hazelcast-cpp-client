//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_SIMPLE_ENTRY_VIEW
#define HAZELCAST_MAP_SIMPLE_ENTRY_VIEW

#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"

namespace hazelcast {
    namespace client {
        /**
         * EntryView represents a readonly view of a map entry.
         *
         * @param <K> key
         * @param <V> value
         */
        template<typename K, typename V>
        class HAZELCAST_API EntryView{
        public:

            /**
             * Constructor
             */
            EntryView(const K &key, const V &value, const map::DataEntryView& rhs)
            : key(key)
            , value(value)
            , cost (rhs.cost)
            , creationTime (rhs.creationTime)
            , expirationTime (rhs.expirationTime)
            , hits (rhs.hits)
            , lastAccessTime (rhs.lastAccessTime)
            , lastStoredTime (rhs.lastStoredTime)
            , lastUpdateTime (rhs.lastUpdateTime)
            , version (rhs.version) {

            };
            /**
             * key
             */
            K key;
            /**
             * value
             */
            V value;
            /**
             * memory cost of entry
             */
            long cost;
            /**
             * entry creation time
             */
            long creationTime;
            /**
             * entry expiration time if ttl is defined.
             */
            long expirationTime;
            /**
             * number of hits.
             */
            long hits;
            /**
             * last access time
             */
            long lastAccessTime;
            /**
             * last stored time.
             */
            long lastStoredTime;
            /**
             * last update time.
             */
            long lastUpdateTime;
            /**
             * version.
             */
            long version;
        };
    }
}

#endif //HAZELCAST_MAP_SIMPLE_ENTRY_VIEW

