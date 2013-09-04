//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "PutAllRequest.h"
#include "PortableHook.h"
#include "MapEntrySet.h"
#include "PortableReader.h"
#include "PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            PutAllRequest::PutAllRequest(const std::string& name, map::MapEntrySet& entrySet)
            :name(name)
            , entrySet(entrySet) {

            };

            int PutAllRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int PutAllRequest::getClassId() const {
                return PortableHook::PUT_ALL;
            }

            void PutAllRequest::writePortable(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                entrySet.writeData(out);
            };

            void PutAllRequest::readPortable(serialization::PortableReader& reader) {
                name = reader.readUTF("n");
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                entrySet.readData(in);
            };
        }
    }
}

#endif //HAZELCAST_MAP_PUT_ALL_REQUEST

