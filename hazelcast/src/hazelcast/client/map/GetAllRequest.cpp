//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/GetAllRequest.h"
#include "PortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace map {
            GetAllRequest::GetAllRequest(const std::string& name, std::vector<serialization::Data>& keys)
            :name(name)
            , keys(keys) {

            };

            int GetAllRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int GetAllRequest::getClassId() const {
                return PortableHook::GET_ALL;
            }

            void GetAllRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                writer.writeInt("size", keys.size());
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                for (int i = 0; i < keys.size(); i++) {
                    keys[i].writeData(out);
                }
            };

            void GetAllRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                int size = reader.readInt("size");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                keys.resize(size);
                for (int i = 0; i < keys.size(); i++) {
                    keys[i].readData(in);
                }
            };
        }
    }
}

