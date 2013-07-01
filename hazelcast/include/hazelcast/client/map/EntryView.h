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

                int getSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
                };

                int getFactoryId() const {
                    return DataSerializableHook::F_ID;
                };

                int getClassId() const {
                    return DataSerializableHook::ENTRY_VIEW;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << key;
                    writer << value;
                    writer << cost;
                    writer << creationTime;
                    writer << expirationTime;
                    writer << hits;
                    writer << lastAccessTime;
                    writer << lastStoredTime;
                    writer << lastUpdateTime;
                    writer << version;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader >> key;
                    reader >> value;
                    reader >> cost;
                    reader >> creationTime;
                    reader >> expirationTime;
                    reader >> hits;
                    reader >> lastAccessTime;
                    reader >> lastStoredTime;
                    reader >> lastUpdateTime;
                    reader >> version;
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
