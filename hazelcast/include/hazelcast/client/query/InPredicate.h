/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#pragma once
#include <string>
#include <vector>
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
            class InPredicate : public Predicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam value The values to search for
                 */
                InPredicate(const char *attributeName, const std::vector<T> &values)
                        : attrName(attributeName), inValues(values) {
                }

                /**
                 * Value to add to the searched values in the query
                 */
                void add(const T &value) {
                    inValues.push_back(value);
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
                    return impl::predicates::IN_PREDICATE;
                }

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                void writeData(serialization::ObjectDataOutput &out) const {
                    out.writeUTF(&attrName);
                    int len = (int)inValues.size();
                    out.writeInt(len);
                    for (int i = 0; i < len; ++i) {
                        out.writeObject<T>(&inValues[i]);
                    }
                }

                /**
                 *Defines how this class will be read.
                 * @param reader ObjectDataInput
                 */
                void readData(serialization::ObjectDataInput &in) {
                    // Not need to read at the client side
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("InPredicate::readData",
                                                                                     "Client should not need to use readData method!!!"));
                }

            private:
                std::string attrName;
                std::vector<T> inValues;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

