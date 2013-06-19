//
// Created by sancar koyunlu on 5/23/13.
// Copyright (c) 2013 hazelcast. All rights reserved.
#ifndef HAZELCAST_MAP_SIMPLE_ENTRY_VIEW
#define HAZELCAST_MAP_SIMPLE_ENTRY_VIEW

#include "../serialization/Data.h"
#include "PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            template<typename K, typename V>
            class EntryView {
            public:
                EntryView(const K& key, const V& value, const EntryView<serialization::Data, serialization::Data>& rhs)
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

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                };

                int getClassId() const {
                    return PortableHook::ENTRY_VIEW;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer["c"] << cost;
                    writer["cr"] << creationTime;
                    writer["ex"] << expirationTime;
                    writer["h"] << hits;
                    writer["lat"] << lastAccessTime;
                    writer["lst"] << lastStoredTime;
                    writer["lut"] << lastUpdateTime;
                    writer["v"] << version;
                    writer << key;
                    writer << value;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader["c"] >> cost;
                    reader["cr"] >> creationTime;
                    reader["ex"] >> expirationTime;
                    reader["h"] >> hits;
                    reader["lat"] >> lastAccessTime;
                    reader["lst"] >> lastStoredTime;
                    reader["lut"] >> lastUpdateTime;
                    reader["v"] >> version;
                    reader >> key;
                    reader >> value;
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
}

#endif //HAZELCAST_MAP_SIMPLE_ENTRY_VIEW
