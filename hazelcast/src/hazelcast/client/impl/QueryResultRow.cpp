//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/QueryResultRow.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/map/DataSerializableHook.h"


namespace hazelcast {
    namespace client {
        namespace impl {

            int QueryResultRow::getFactoryId() const {
                return map::DataSerializableHook::F_ID;
            }

            int QueryResultRow::getClassId() const {
                return map::DataSerializableHook::QUERY_RESULT_ROW;
            }

            void QueryResultRow::readData(serialization::ObjectDataInput& reader) {
                key = reader.readData();
                value = reader.readData();
            }
        }
    }
}
