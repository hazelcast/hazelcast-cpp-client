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
        template<typename K, typename V>
        class HAZELCAST_API EntryView : public IdentifiedDataSerializable {
        public:
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

            int getFactoryId() const {
                return map::DataSerializableHook::F_ID;
            };

            int getClassId() const {
                return map::DataSerializableHook::ENTRY_VIEW;
            };

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
            K key;
            V value;
        private:
            long cost;
            long creationTime;
            long expirationTime;
            long hits;
            long lastAccessTime;
            long lastStoredTime;
            long lastUpdateTime;
            long version;
        };
    }
}

#endif //HAZELCAST_MAP_SIMPLE_ENTRY_VIEW
