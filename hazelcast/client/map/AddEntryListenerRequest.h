//
// Created by sancar koyunlu on 6/21/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ADD_ENTRY_LISTENER_REQUEST
#define HAZELCAST_ADD_ENTRY_LISTENER_REQUEST

#include "PortableHook.h"
#include "../serialization/SerializationConstants.h"
#include "Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace map {

            class AddEntryListenerRequest {
            public:
                AddEntryListenerRequest(const std::string& name, bool includeValue)
                :name(name), includeValue(includeValue), hasKey(false), hasPredicate(false) {

                }

                AddEntryListenerRequest(const std::string& name, bool includeValue, const serialization::Data& key, const std::string& sql)
                :name(name), includeValue(includeValue), key(key), sql(sql), hasKey(true), hasPredicate(true) {

                };

                AddEntryListenerRequest(const std::string& name, bool includeValue, const std::string& sql)
                :name(name), includeValue(includeValue), sql(sql), hasPredicate(true) {

                };

                AddEntryListenerRequest(const std::string& name, bool includeValue, const serialization::Data& key)
                :name(name), includeValue(includeValue), key(key), hasKey(true) {

                };

                int getTypeSerializerId() const {
                    return serialization::SerializationConstants::CONSTANT_TYPE_PORTABLE;
                };

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::ADD_ENTRY_LISTENER;
                }

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer["name"] << name;
                    writer["i"] << includeValue;
                    writer["key"] << hasKey;
                    writer["pre"] << hasPredicate;
                    if (hasPredicate) {
                        writer["p"] << sql;
                    }
                    if (hasKey) {
                        writer << key;
                    }
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    reader["name"] >> name;
                    reader["i"] >> includeValue;
                    reader["key"] >> hasKey;
                    reader["pre"] >> hasPredicate;
                    if (hasPredicate) {
                        reader["p"] >> sql;
                    }
                    if (hasKey) {
                        reader >> key;
                    }
                };
            private:
                std::string name;
                std::string sql;
                serialization::Data key;
                bool includeValue;
                bool hasKey;
                bool hasPredicate;
            };

        }
    }
}

#endif //HAZELCAST_ADD_ENTRY_LISTENER_REQUEST
