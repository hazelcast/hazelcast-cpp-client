//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_SIMPLE_ENTRY_VIEW
#define HAZELCAST_MAP_SIMPLE_ENTRY_VIEW

#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        /**
         * EntryView represents a readonly view of a map entry.
         *
         * @param <K> key
         * @param <V> value
         */
        template<typename K, typename V>
        class HAZELCAST_API EntryView : public IdentifiedDataSerializable {
        public:
            /**
             * Constrcutor
             */
            EntryView(const K &key, const V &value, const EntryView<serialization::Data, serialization::Data> &rhs)
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
             * @see IdentifiedDataSerializable
             */
            int getFactoryId() const {
                return map::DataSerializableHook::F_ID;
            };
            /**
             * @see IdentifiedDataSerializable
             */
            int getClassId() const {
                return map::DataSerializableHook::ENTRY_VIEW;
            };
            /**
             * @see IdentifiedDataSerializable
             * @param out
             */
            void writeData(serialization::ObjectDataOutput &out) const {
                out.writeObject(&key);
                out.writeObject(&value);
                out.writeLong(cost);
                out.writeLong(creationTime);
                out.writeLong(expirationTime);
                out.writeLong(hits);
                out.writeLong(lastAccessTime);
                out.writeLong(lastStoredTime);
                out.writeLong(lastUpdateTime);
                out.writeLong(version);
            };
            /**
             * @see IdentifiedDataSerializable
             * @param in
             */
            void readData(serialization::ObjectDataInput &in) {
                key = in.readObject<K>();
                value = in.readObject<V>();
                cost = in.readLong();
                creationTime = in.readLong();
                expirationTime = in.readLong();
                hits = in.readLong();
                lastAccessTime = in.readLong();
                lastStoredTime = in.readLong();
                lastUpdateTime = in.readLong();
                version = in.readLong();
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
             * entry creadion time
             */
            long creationTime;
            /**
             * entry expriation time if ttl is defined.
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
