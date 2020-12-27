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
#include <ostream>

#include "hazelcast/util/export.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        /**
         * hazelcast_json_value is a wrapper for Json formatted strings. It is preferred
         * to store hazelcast_json_value instead of std::string for Json formatted strings.
         * Users can run predicates and use indexes on the attributes of the underlying
         * Json strings.
         *
         * hazelcast_json_value is queried using Hazelcast's querying language.
         * See {@link query::Predicate}.
         *
         * In terms of querying, numbers in Json strings are treated as either
         * {@code int64_t} or {@code double}. Strings, bools and NULL are treated as
         * their C++ counterparts.
         *
         * hazelcast_json_value keeps given string as it is.
         *
         */
        class HAZELCAST_API hazelcast_json_value {
        public:
            /**
             * Create a hazelcast_json_value from a string. This method does not the check
             * validity of the underlying Json string. Invalid Json strings may cause
             * wrong results in queries.
             *
             * @param jsonString The json string
             * @return The hazelcast_json_value representing the json string.
             */
            hazelcast_json_value(std::string json_string);

            virtual ~hazelcast_json_value();

            /**
             * This method returns a Json representation of the object
             * @return Json string representation of the object
             */
            const std::string &to_string() const;

            bool operator==(const hazelcast_json_value &rhs) const;

            bool operator!=(const hazelcast_json_value &rhs) const;

            bool operator<(const hazelcast_json_value &rhs) const;

            friend std::ostream HAZELCAST_API &operator<<(std::ostream &os, const hazelcast_json_value &value);

        private:
            std::string json_string_;
        };
    }
}

namespace std {
    template<> struct hash<hazelcast::client::hazelcast_json_value> {
        std::size_t operator()(const hazelcast::client::hazelcast_json_value &object) const noexcept;
    };
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



