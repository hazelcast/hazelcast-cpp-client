//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/AddEntryListenerRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/EntryEvent.h"


namespace hazelcast {
    namespace client {
        namespace map {

            int AddEntryListenerRequest::ALL_LISTENER_FLAGS =  EntryEventType::ADDED |
                                                                EntryEventType::REMOVED  |
                                                                EntryEventType::UPDATED  |
                                                                EntryEventType::EVICTED  |
                                                                EntryEventType::EVICT_ALL |
                                                                EntryEventType::CLEAR_ALL |
                                                                EntryEventType::MERGED |
                                                                EntryEventType::EXPIRED;

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue)
            :name(name), includeValue(includeValue), hasKey(false), hasPredicate(false) {

            }

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue, const serialization::pimpl::Data &key, const std::string &sql)
            :name(name), includeValue(includeValue), key(key), sql(sql), hasKey(true), hasPredicate(true) {

            }

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue, const std::string &sql)
            :name(name), includeValue(includeValue), sql(sql), hasKey(false), hasPredicate(true) {

            }

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue, const serialization::pimpl::Data &key)
            :name(name), includeValue(includeValue), key(key), hasKey(true), hasPredicate(false) {

            }

            int AddEntryListenerRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int AddEntryListenerRequest::getClassId() const {
                return PortableHook::ADD_ENTRY_LISTENER_SQL;
            }

            const serialization::pimpl::Data *AddEntryListenerRequest::getKey() const {
                return &key;
            }

            void AddEntryListenerRequest::write(serialization::PortableWriter &writer) const {
                writer.writeBoolean("l", false);
                writer.writeUTF("name", &name);
                writer.writeBoolean("i", includeValue);
                writer.writeInt("lf", ALL_LISTENER_FLAGS);
                writer.writeBoolean("key", hasKey);
                writer.writeBoolean("pre", hasPredicate);
                if (hasPredicate) {
                    writer.writeUTF("p", &sql);
                }
                if (hasKey) {
                    serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                    out.writeData(&key);
                }
            }


        }
    }
}

