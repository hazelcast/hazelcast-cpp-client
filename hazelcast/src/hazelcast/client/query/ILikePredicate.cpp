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

#include "hazelcast/client/query/ILikePredicate.h"
#include "hazelcast/client/query/impl/predicates/PredicateDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        namespace query {
            ILikePredicate::ILikePredicate(const std::string &attribute, const std::string &expression) : attributeName(
                    attribute), expressionString(expression) {
            }

            int ILikePredicate::getFactoryId() const {
                return impl::predicates::F_ID;
            }

            int ILikePredicate::getClassId() const {
                return impl::predicates::ILIKE_PREDICATE;
            }

            void ILikePredicate::writeData(serialization::ObjectDataOutput &out) const {
                out.writeUTF(&attributeName);
                out.writeUTF(&expressionString);
            }

            void ILikePredicate::readData(serialization::ObjectDataInput &in) {
                // Not need to read at the client side
                throw exception::IException("ILikePredicate::readData",
                                            "Client should not need to use readData method!!!");
            }
        }
    }
}
