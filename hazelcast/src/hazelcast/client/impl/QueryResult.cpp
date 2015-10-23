/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
// Created by sancar koyunlu on 9/2/13.



#include "hazelcast/client/impl/QueryResult.h"
#include "hazelcast/client/impl/QueryResultRow.h"
#include "hazelcast/client/map/DataSerializableHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"


namespace hazelcast {
    namespace client {
        namespace impl {
            int QueryResult::getFactoryId() const {
                return map::DataSerializableHook::F_ID;
            }

            int QueryResult::getClassId() const {
                return map::DataSerializableHook::QUERY_RESULT;
            }

            std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > QueryResult::getResultData() const {
                return q;
            }

            void QueryResult::readData(serialization::ObjectDataInput &in) {
                int partitionSize = in.readInt();
                for (int i = 0; i < partitionSize; i++) {
                    in.readInt(); // not used in C++ client.
                }
                in.readByte(); // iteration type
                int size = in.readInt();
                for (int i = 0; i < size; i++) {
                    QueryResultRow entry;
                    entry.readData(in);
                    q.push_back(std::make_pair(entry.key, entry.value));
                }

            }
        }
    }
}
