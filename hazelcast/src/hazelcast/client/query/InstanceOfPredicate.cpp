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

#include "hazelcast/client/query/InstanceOfPredicate.h"
#include "hazelcast/client/query/impl/predicates/PredicateDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace query {
            InstanceOfPredicate::InstanceOfPredicate(const char *javaClassName) : className(javaClassName) {
            }

            int InstanceOfPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int InstanceOfPredicate::getClassId() const {
                return impl::predicates::INSTANCEOF_PREDICATE;
            }

            void InstanceOfPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&className);
            }

            void InstanceOfPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::IException("InstanceOfPredicate::readData",
                                            "Client should not need to use readData method!!!");
            }
        }
    }
}
