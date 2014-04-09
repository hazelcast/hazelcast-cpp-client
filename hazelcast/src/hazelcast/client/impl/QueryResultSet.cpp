//
// Created by sancar koyunlu on 9/2/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/impl/QueryResultSet.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"


namespace hazelcast {
    namespace client {
        namespace impl {
            int QueryResultSet::getFactoryId() const {
                return map::DataSerializableHook::F_ID;
            }

            int QueryResultSet::getClassId() const {
                return map::DataSerializableHook::QUERY_RESULT_SET;
            };

            const std::vector<QueryResultEntry> &QueryResultSet::getResultData() const {
                return q;
            };

            void QueryResultSet::readData(serialization::ObjectDataInput &in) {
                in.readBoolean(); //isData
                in.readUTF(); //iterationType
                int size = in.readInt();
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<QueryResultEntry> entry = in.readObject<QueryResultEntry>();
                    q.push_back(*entry);
                }

            };
        }
    }
}
