//
// Created by sancar koyunlu on 9/4/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/map/ContainsValueRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/map/PortableHook.h"

namespace hazelcast {
    namespace client {
        namespace map {
            ContainsValueRequest::ContainsValueRequest(const std::string &name, const serialization::pimpl::Data &value)
            :name(name)
            , value(value) {
            }

            int ContainsValueRequest::getFactoryId() const {
                return PortableHook::F_ID;
            }

            int ContainsValueRequest::getClassId() const {
                return PortableHook::CONTAINS_VALUE;
            }

            void ContainsValueRequest::write(serialization::PortableWriter &writer) const {
                writer.writeUTF("n", &name);
                serialization::ObjectDataOutput &out = writer.getRawDataOutput();
                out.writeData(&value);
            }

        }
    }
}


