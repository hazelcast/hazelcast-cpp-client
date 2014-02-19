//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/QueryResultEntry.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/map/DataSerializableHook.h"


namespace hazelcast {
    namespace client {
        namespace impl {

            int QueryResultEntry::getFactoryId() const {
                return map::DataSerializableHook::F_ID;
            }

            int QueryResultEntry::getClassId() const {
                return map::DataSerializableHook::QUERY_RESULT_ENTRY;
            }

            void QueryResultEntry::readData(serialization::ObjectDataInput& reader) {
                bool isNotNull = reader.readBoolean();
                if (isNotNull)
                    keyIndex.readData(reader);
                isNotNull = reader.readBoolean();
                if (isNotNull)
                    key.readData(reader);
                isNotNull = reader.readBoolean();
                if (isNotNull)
                    value.readData(reader);
            };
        }
    }
}