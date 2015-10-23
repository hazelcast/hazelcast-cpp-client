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



#include "hazelcast/client/collection/ListSetRequest.h"
#include "hazelcast/client/serialization/PortableWriter.h"
#include "hazelcast/client/collection/CollectionPortableHook.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace list {
            ListSetRequest::ListSetRequest(const std::string &name, const std::string &serviceName, const serialization::pimpl::Data &data, int index)
            : CollectionRequest(name, serviceName)
            , data(data)
            , index(index) {

            }

            void ListSetRequest::write(serialization::PortableWriter &writer) const {
                CollectionRequest::write(writer);
                writer.writeInt("i", index);
                serialization::ObjectDataOutput &output = writer.getRawDataOutput();
                output.writeData(&data);
            }

            int ListSetRequest::getClassId() const {
                return collection::CollectionPortableHook::LIST_SET;
            }
        }
    }
}
