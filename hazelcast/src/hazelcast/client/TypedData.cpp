/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/TypedData.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        TypedData::TypedData() : ss(NULL) {
        }

        TypedData::TypedData(std::auto_ptr<serialization::pimpl::Data> data,
                             serialization::pimpl::SerializationService &serializationService) : data(data),
                                                                                                 ss(&serializationService) {
        }

        TypedData::TypedData(const boost::shared_ptr<serialization::pimpl::Data> &data,
                             serialization::pimpl::SerializationService &serializationService) : data(data),
                                                                                                 ss(&serializationService) {

        }

        TypedData::~TypedData() {}

        const serialization::pimpl::ObjectType TypedData::getType() const {
            return ss->getObjectType(data.get());
        }

        const boost::shared_ptr<serialization::pimpl::Data> TypedData::getData() const {
            return data;
        }

        bool operator<(const TypedData &lhs, const TypedData &rhs) {
            const serialization::pimpl::Data *lhsData = lhs.getData().get();
            const serialization::pimpl::Data *rhsData = rhs.getData().get();
            if (lhsData == NULL) {
                return true;
            }

            if (rhsData == NULL) {
                return false;
            }

            return *lhsData < *rhsData;
        }
    }
}
