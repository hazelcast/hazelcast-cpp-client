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

#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/serialization.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        /**
         * TypedData class is a wrapper class for the serialized binary data. It does late deserialization of the data
         * only when the get method is called.
         */
        class HAZELCAST_API TypedData {
        public:
            TypedData();

            TypedData(serialization::pimpl::Data d,
                      serialization::pimpl::SerializationService &serializationService);

            /**
             *
             * @return The type of the underlying object for this binary.
             */
            serialization::pimpl::ObjectType getType() const;

            /**
             * Deserializes the underlying binary data and produces the object of type T.
             *
             * <b>CAUTION</b>: The type that you provide should be compatible with what object type is returned with
             * the getType API, otherwise you will either get an exception of incorrectly try deserialize the binary data.
             *
             * @tparam T The type to be used for deserialization
             * @return The object instance of type T.
             */
            template <typename T>
            boost::optional<T> get() const {
                return ss->toObject<T>(data);
            }

            /**
             * Internal API
             * @return The pointer to the internal binary data.
             */
            const serialization::pimpl::Data &getData() const;

        private:
            serialization::pimpl::Data data;
            serialization::pimpl::SerializationService *ss;
        };

        bool HAZELCAST_API operator<(const TypedData &lhs, const TypedData &rhs);
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

