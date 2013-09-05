//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "GetLockCountRequest.h"
#include "Data.h"
#include "LockPortableHook.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            GetLockCountRequest::GetLockCountRequest(serialization::Data& key)
            :key(key) {
            };

            int GetLockCountRequest::getClassId() const {
                return LockPortableHook::GET_LOCK_COUNT;
            };

            int GetLockCountRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            };

            void GetLockCountRequest::writePortable(serialization::PortableWriter& writer) const {
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void GetLockCountRequest::readPortable(serialization::PortableReader& reader) {
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
