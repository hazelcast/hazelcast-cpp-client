//
// Created by sancar koyunlu on 9/10/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/multimap/AddEntryListenerRequest.h"
#include "hazelcast/client/multimap/MultiMapPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/util/Util.h"

namespace hazelcast {
    namespace client {
        namespace multimap {
            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, const serialization::pimpl::Data &key, bool includeValue)
            :name(name)
            , key(key)
            , hasKey(true)
            , includeValue(includeValue) {

            }

            AddEntryListenerRequest::AddEntryListenerRequest(const std::string &name, bool includeValue)
            :name(name)
            , hasKey(false)
            , includeValue(includeValue) {

            }

            int AddEntryListenerRequest::getFactoryId() const {
                return MultiMapPortableHook::F_ID;
            }

            int AddEntryListenerRequest::getClassId() const {
                return MultiMapPortableHook::ADD_ENTRY_LISTENER;
            }


            void AddEntryListenerRequest::write(serialization::PortableWriter &writer) const {
                writer.writeBoolean("l", false);
                writer.writeBoolean("i", includeValue);
                writer.writeUTF("n", &name);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                
                if (hasKey) {
                    out.writeData(&key);
                } else {
                    out.writeData(NULL);
                }
            }

            bool AddEntryListenerRequest::isRetryable() const {
                return true;
            }

        }
    }
}


