//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "GetEntryViewRequest.h"
#include "PortableHook.h"
#include "Data.h"
#include "PortableReader.h"
#include "PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            GetEntryViewRequest::GetEntryViewRequest(const std::string& name, serialization::Data& key)
            :name(name)
            , key(key) {

            };

            int GetEntryViewRequest::getFactoryId() const {
                return PortableHook::F_ID;
            };

            int GetEntryViewRequest::getClassId() const {
                return PortableHook::GET_ENTRY_VIEW;
            };


            void GetEntryViewRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };


            void GetEntryViewRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}

