//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "RemoveRequest.h"
#include "QueuePortableHook.h"
#include "Data.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace queue {
            RemoveRequest::RemoveRequest(const std::string& name, serialization::Data& data)
            :name(name)
            , data(data) {

            };

            int RemoveRequest::getFactoryId() const {
                return QueuePortableHook::F_ID;
            }

            int RemoveRequest::getClassId() const {
                return QueuePortableHook::REMOVE;
            }


            void RemoveRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                data.writeData(out);
            };


            void RemoveRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                data.readData(in);
            };
        }
    }
}
