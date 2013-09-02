//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "QueryResultEntry.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include "hazelcast/client/map/DataSerializableHook.h"


namespace hazelcast {
    namespace client {
        namespace impl {
            QueryResultEntry::QueryResultEntry() {

            };

            int QueryResultEntry::getFactoryId() const {
                return map::DataSerializableHook::F_ID;
            }

            int QueryResultEntry::getClassId() const {
                return map::DataSerializableHook::QUERY_RESULT_ENTRY;
            }

            void QueryResultEntry::writeData(serialization::ObjectDataOutput& writer) const {
                writer.writeBoolean(true);
                keyIndex.writeData(writer);
                writer.writeBoolean(true);
                key.writeData(writer);
                writer.writeBoolean(true);
                value.writeData(writer);
            };

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