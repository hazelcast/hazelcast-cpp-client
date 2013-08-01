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

            class AddEntryListenerRequest : public Portable {
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

                int getFactoryId() const {
                    return PortableHook::F_ID;
                }

                int getClassId() const {
                    return PortableHook::ADD_ENTRY_LISTENER;
                }

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const {
                    writer.writeUTF("name", name);
                    writer.writeBoolean("i", includeValue);
                    writer.writeBoolean("key", hasKey);
                    writer.writeBoolean("pre", hasPredicate);
                    if (hasPredicate) {
                        writer.writeUTF("p", sql);
                    }
                    if (hasKey) {
                        serialization::ObjectDataOutput *out = writer.getRawDataOutput();
                        key.writeData(*out);
                    }
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    name = reader.readUTF("name");
                    includeValue = reader.readBoolean("i");
                    hasKey = reader.readBoolean("key");
                    hasPredicate = reader.readBoolean("pre");
                    if (hasPredicate) {
                        sql = reader.readUTF("p");
                    }
                    if (hasKey) {
                        serialization::ObjectDataInput *in = reader.getRawDataInput();
                        key.readData(*in);
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
