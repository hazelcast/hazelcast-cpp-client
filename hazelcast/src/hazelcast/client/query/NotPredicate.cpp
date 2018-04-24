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

#include "hazelcast/client/query/NotPredicate.h"
#include "hazelcast/client/query/impl/predicates/PredicateDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace query {
            NotPredicate::NotPredicate(std::auto_ptr<Predicate> predicate)
                    : internalPredicate(predicate) {
            }

            int NotPredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int NotPredicate::getClassId() const {
                return impl::predicates::NOT_PREDICATE;
            }

            void NotPredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeObject<serialization::IdentifiedDataSerializable>(internalPredicate.get());
            }

            void NotPredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::HazelcastSerializationException("NotPredicate::readData",
                                            "Client should not need to use readData method!!!");
            }
        }
    }
}
