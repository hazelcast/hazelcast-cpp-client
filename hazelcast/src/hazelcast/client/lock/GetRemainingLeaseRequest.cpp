//
// Created by sancar koyunlu on 9/5/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "GetRemainingLeaseRequest.h"
#include "hazelcast/client/serialization/Data.h"
#include "LockPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace lock {
            GetRemainingLeaseRequest::GetRemainingLeaseRequest(serialization::Data& key)
            :key(key) {
            };

            int GetRemainingLeaseRequest::getClassId() const {
                return LockPortableHook::GET_REMAINING_LEASE;
            };

            int GetRemainingLeaseRequest::getFactoryId() const {
                return LockPortableHook::FACTORY_ID;
            };

            void GetRemainingLeaseRequest::writePortable(serialization::PortableWriter& writer) const {
                serialization::ObjectDataOutput& out = writer.getRawDataOutput();
                key.writeData(out);
            };

            void GetRemainingLeaseRequest::readPortable(serialization::PortableReader& reader) {
                serialization::ObjectDataInput &in = reader.getRawDataInput();
                key.readData(in);
            };
        }
    }
}
