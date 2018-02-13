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
#ifndef HAZELCAST_CLIENT_QUERY_BETWEENPREDICATE_H_
#define HAZELCAST_CLIENT_QUERY_BETWEENPREDICATE_H_

#include <string>
#include <memory>
#include "hazelcast/client/query/Predicate.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/exception/IException.h"
#include "hazelcast/client/query/impl/predicates/PredicateDataSerializerHook.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace query {
            /**
             * Type T should be a valid serializable and copiable type.
             */
            template <typename T>
            class BetweenPredicate : public Predicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam from The starting value to match (start is inclusive).
                 * @tparam to The ending value to match (end is inclusive).
                 */
                BetweenPredicate(const char *attributeName, const T &from, const T &to)
                        : attrName(attributeName), begin(from), end(to) {
                }

                /**
                 * @return factory id
                 */
                int getFactoryId() const {
                    return impl::predicates::F_ID;
                }

                /**
                 * @return class id
                 */
                int getClassId() const {
                    return impl::predicates::BETWEEN_PREDICATE;
                }

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                void writeData(serialization::ObjectDataOutput &out) const {
                    out.writeUTF(&attrName);
                    out.writeObject<T>(&end);
                    out.writeObject<T>(&begin);
                }

                /**
                 *Defines how this class will be read.
                 * @param reader ObjectDataInput
                 */
                void readData(serialization::ObjectDataInput &in) {
                    // Not need to read at the client side
                    throw exception::IException("BetweenPredicate::readData",
                                                "Client should not need to use readData method!!!");
                }

            private:
                std::string attrName;
                T begin;
                T end;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_CLIENT_QUERY_BETWEENPREDICATE_H_ */
