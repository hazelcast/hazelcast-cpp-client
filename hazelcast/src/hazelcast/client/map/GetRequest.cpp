//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/GetRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"


namespace hazelcast {
    namespace client {
        namespace map {
            GetRequest::GetRequest(std::string& name, serialization::Data& key)
            :name(name)
            , key(key) {

            };

            int GetRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int GetRequest::getClassId() const {
                return PortableHook::GET;
            }

            void GetRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void GetRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
