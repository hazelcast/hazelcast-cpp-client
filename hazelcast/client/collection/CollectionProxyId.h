//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CollectionProxyId
#define HAZELCAST_CollectionProxyId

#include <string>
#include "../serialization/SerializationConstants.h"
#include "CollectionDataSerializerHook.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            class CollectionProxyId {
            public:
                enum CollectionProxyType {
                    MULTI_MAP, LIST, SET
                };

                CollectionProxyId(const std::string& name, CollectionProxyType type)
                :name(name)
                , keyName("")
                , type(type) {
                };

                CollectionProxyId(const std::string& name, const std::string& keyName, CollectionProxyType type)
                :name(name)
                , keyName(keyName)
                , type(type) {
                };

                std::string getName() const {
                    return name;
                };

                std::string getKeyName() const {
                    return keyName;
                };

                CollectionProxyType getType() const {
                    return type;
                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_DATA;
                };

                int getFactoryId() const {
                    return CollectionDataSerializerHook::F_ID;
                };

                int getClassId() const {
                    return CollectionDataSerializerHook::COLLECTION_PROXY_ID;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    writer << name;
                    writer << (int) type;
                    writer << keyName;
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    reader >> name;
                    int type;
                    reader >> type;
                    reader >> keyName;
                };

            private:
                std::string name;
                std::string keyName;
                CollectionProxyType type;
            };
        }
    }
}

#endif //HAZELCAST_CollectionProxyId
