//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/AddEntryListenerRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/SerializationConstants.h"
#include "hazelcast/client/serialization/PortableWriter.h"


namespace hazelcast {
    namespace client {
        namespace map {


            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue)
            :name(name), includeValue(includeValue), hasKey(false), hasPredicate(false) {

            }

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue, const serialization::Data &key, const std::string &sql)
            :name(name), includeValue(includeValue), key(key), sql(sql), hasKey(true), hasPredicate(true) {

            };

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue, const std::string &sql)
            :name(name), includeValue(includeValue), sql(sql), hasKey(false), hasPredicate(true) {

            };

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue, const serialization::Data &key)
            :name(name), includeValue(includeValue), key(key), hasKey(true), hasPredicate(false) {

            };

            int AddEntryListenerRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int AddEntryListenerRequest::getClassId() const {
                return PortableHook::ADD_ENTRY_LISTENER_SQL;
            }

            const serialization::Data *AddEntryListenerRequest::getKey() const {
                return &key;
            }

            void AddEntryListenerRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("name", name);
                writer.writeBoolean("i", includeValue);
                writer.writeBoolean("key", hasKey);
                writer.writeBoolean("pre", hasPredicate);
                if (hasPredicate) {
                    writer.writeUTF("p", sql);
                }
                if (hasKey) {
                    serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                    key.writeData(out);
                }
            };


        }
    }
}
