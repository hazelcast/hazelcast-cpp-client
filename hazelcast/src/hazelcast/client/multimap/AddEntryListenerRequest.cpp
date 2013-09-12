//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/AddEntryListenerRequest.h"
#include "MultiMapPortableHook.h"
#include "PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            AddEntryListenerRequest::AddEntryListenerRequest(const std::string& name, const serialization::Data& key, bool includeValue)
            :name(name)
            , key(&key)
            , includeValue(includeValue) {

            };

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string& name, bool includeValue)
            :name(name)
            , key(NULL)
            , includeValue(includeValue) {

            };

            int AddEntryListenerRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            };

            int AddEntryListenerRequest::getClassId() const {
                return MultiMapPortableHook::ADD_ENTRY_LISTENER;
            };


            void AddEntryListenerRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeBoolean("i", includeValue);
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                util::writeNullableData(out, key);
            };

        }
    }
}

