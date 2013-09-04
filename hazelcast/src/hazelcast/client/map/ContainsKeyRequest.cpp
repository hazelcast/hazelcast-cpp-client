//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.

#include "ContainsKeyRequest.h"
#include "PortableHook.h"
#include "Data.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            ContainsKeyRequest::ContainsKeyRequest(const std::string& name, serialization::Data& key)
            :name(name)
            , key(key) {
            };

            int ContainsKeyRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ContainsKeyRequest::getClassId() const {
                return PortableHook::CONTAINS_KEY;
            }


            void ContainsKeyRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };


            void ContainsKeyRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
