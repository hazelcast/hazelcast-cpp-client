//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/lock/DestroyRequest.h"
#include "Data.h"
#include "LockPortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            DestroyRequest::DestroyRequest(serialization::Data& key)
            :key(key) {
            };

            int DestroyRequest::getClassId() const {
                return LockPortableHook::GET_LOCK_COUNT;
            };

            int DestroyRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            };

            void DestroyRequest::writePortable(serialization::PortableWriter& writer) const {
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void DestroyRequest::readPortable(serialization::PortableReader& reader) {
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
