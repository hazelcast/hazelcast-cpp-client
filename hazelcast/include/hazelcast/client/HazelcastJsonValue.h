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

#ifndef HAZELCAST_CLIENT_SERIALIZATION_JSON_HAZELCASTJSONVALUE_H
#define HAZELCAST_CLIENT_SERIALIZATION_JSON_HAZELCASTJSONVALUE_H

#include <string>
#include <ostream>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * HazelcastJsonValue is a wrapper for Json formatted strings. It is preferred
         * to store HazelcastJsonValue instead of std::string for Json formatted strings.
         * Users can run predicates and use indexes on the attributes of the underlying
         * Json strings.
         *
         * HazelcastJsonValue is queried using Hazelcast's querying language.
         * See {@link query::Predicate}.
         *
         * In terms of querying, numbers in Json strings are treated as either
         * {@code int64_t} or {@code double}. Strings, bools and NULL are treated as
         * their C++ counterparts.
         *
         * HazelcastJsonValue keeps given string as it is.
         *
         */
        class HAZELCAST_API HazelcastJsonValue {
        public:
            /**
             * Create a HazelcastJsonValue from a string. This method does not the check
             * validity of the underlying Json string. Invalid Json strings may cause
             * wrong results in queries.
             *
             * @param jsonString The json string
             * @return The HazelcastJsonValue representing the json string.
             */
            HazelcastJsonValue(const std::string &jsonString);

            virtual ~HazelcastJsonValue();

            /**
             * This method returns a Json representation of the object
             * @return Json string representation of the object
             */
            const std::string &toString() const;

            bool operator==(const HazelcastJsonValue &rhs) const;

            bool operator!=(const HazelcastJsonValue &rhs) const;

            friend std::ostream HAZELCAST_API &operator<<(std::ostream &os, const HazelcastJsonValue &value);

        private:
            std::string jsonString;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SERIALIZATION_JSON_HAZELCASTJSONVALUE_H

