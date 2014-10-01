//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/PutAllRequest.h"
#include "hazelcast/client/map/PortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace map {
            PutAllRequest::PutAllRequest(const std::string& name, const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >& m)
            : name(name)
            , entrySet(m) {

            }

            int PutAllRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int PutAllRequest::getClassId() const {
                return PortableHook::PUT_ALL;
            }

            void PutAllRequest::write(serialization::PortableWriter& writer) const {
                writer.writeUTF("n", name);
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                entrySet.writeData(out);
            }
        }
    }
}


