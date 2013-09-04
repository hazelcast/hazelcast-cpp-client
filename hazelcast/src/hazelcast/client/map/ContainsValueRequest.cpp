//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ContainsValueRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "PortableHook.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace map {
            ContainsValueRequest::ContainsValueRequest(const std::string& name, serialization::Data& value)
            :name(name)
            , value(value) {
            };

            int ContainsValueRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ContainsValueRequest::getClassId() const {
                return PortableHook::CONTAINS_VALUE;
            }


            void ContainsValueRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                value.writeData(out);
            };


            void ContainsValueRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                value.readData(in);
            };
        }
    }
}

