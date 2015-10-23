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
// Created by sancar koyunlu on 9/12/13.



#include "hazelcast/client/collection/CollectionAddAllRequest.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace collection {
            CollectionAddAllRequest::CollectionAddAllRequest(const std::string &name, const std::string &serviceName, const std::vector<serialization::pimpl::Data> &valueList)
            : CollectionRequest(name, serviceName)
            , valueList(valueList) {

            }

            int CollectionAddAllRequest::getClassId() const {
                return CollectionPortableHook::COLLECTION_ADD_ALL;
            }

            void CollectionAddAllRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                serialization::ObjectDataOutput &output = writer.getRawDataOutput();
                int size = valueList.size();
                output.writeInt(size);
                for (int i = 0; i < size; i++) {
                    output.writeData(&(valueList[i]));
                }
            }
        }
    }
}

