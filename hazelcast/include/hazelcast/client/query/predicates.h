/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/serialization/serialization.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class hazelcast_client;

        namespace query {
            struct HAZELCAST_API query_constants {
                static const char * const KEY_ATTRIBUTE_NAME;
                static const char * const THIS_ATTRIBUTE_NAME;
            };

            /**
             * This is a marker class for Predicate classes. All predicate classes shall extend this class. This class
             * also indicates that a Java implementation is required at the server side.
             */
            class HAZELCAST_API predicate {
            };

            struct HAZELCAST_API base_predicate : public predicate {
                explicit base_predicate(hazelcast_client &client);

                serialization::object_data_output out_stream;
            };

            class HAZELCAST_API named_predicate : public base_predicate {
            protected:
                explicit named_predicate(hazelcast_client &client, const std::string &attribute_name);
            };

            class multi_predicate : public base_predicate {
            public:
                template<typename ...Args>
                multi_predicate(hazelcast_client &client, const Args &...values) : base_predicate(client) {
                    out_stream.write<int32_t>(static_cast<int32_t>(sizeof...(values)));
                    out_stream.write_objects(values...);
                }

                template<typename ...Args>
                multi_predicate(const std::string attribute_name, hazelcast_client &client, const Args &...values) : base_predicate(client) {
                    out_stream.write(attribute_name);
                    out_stream.write<int32_t>(static_cast<int32_t>(sizeof...(values)));
                    out_stream.write_objects(values...);
                }

                template<typename T>
                multi_predicate(const std::string attribute_name, hazelcast_client &client, const std::vector<T> &values) : base_predicate(client) {
                    out_stream.write(attribute_name);
                    out_stream.write<int32_t>(static_cast<int32_t>(values.size()));
                    for (const T &value : values) {
                        out_stream.write_object(value);
                    }
                }
            };

            enum struct predicate_data_serializer_hook {
                F_ID = -20,

                SQL_PREDICATE = 0,

                AND_PREDICATE = 1,

                BETWEEN_PREDICATE = 2,

                EQUAL_PREDICATE = 3,

                GREATERLESS_PREDICATE = 4,

                LIKE_PREDICATE = 5,

                ILIKE_PREDICATE = 6,

                IN_PREDICATE = 7,

                INSTANCEOF_PREDICATE = 8,

                NOTEQUAL_PREDICATE = 9,

                NOT_PREDICATE = 10,

                OR_PREDICATE = 11,

                REGEX_PREDICATE = 12,

                FALSE_PREDICATE = 13,

                TRUE_PREDICATE = 14,

                PAGING_PREDICATE = 15
            };

            class equal_predicate : public named_predicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam from The value of the attribute.
                 */
                template<typename T>
                equal_predicate(hazelcast_client &client, const std::string &attribute_name, const T &value)
                        : named_predicate(client, attribute_name) {
                    out_stream.write_object(value);
                }
            };

            class not_equal_predicate : public named_predicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam from The value of the attribute.
                 */
                template<typename T>
                not_equal_predicate(hazelcast_client &client, const std::string &attribute_name, const T &value)
                        : named_predicate(client, attribute_name) {
                    out_stream.write_object(value);
                }
            };

            class greater_less_predicate : public named_predicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam value The value to compare
                 * @param equal Allow equal matching
                 * @param less If true, allow "less than" matching otherwise do "greater than" matching
                 */
                template<typename T>
                greater_less_predicate(hazelcast_client &client, const std::string &attribute_name, const T &value,
                                       bool is_equal, bool is_less)
                        : named_predicate(client, attribute_name) {
                    out_stream.write_object(value);
                    out_stream.write(is_equal);
                    out_stream.write(is_less);
                }
            };

            class between_predicate : public named_predicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam from The starting value to match (start is inclusive).
                 * @tparam to The ending value to match (end is inclusive).
                 */
                template<typename FROM_TYPE, typename TO_TYPE>
                between_predicate(hazelcast_client &client, const std::string &attribute_name, const FROM_TYPE &from,
                                  const TO_TYPE &to)
                        : named_predicate(client, attribute_name) {
                    out_stream.write_object(to);
                    out_stream.write_object(from);
                }
            };

            class HAZELCAST_API false_predicate : public base_predicate {
            public:
                false_predicate(hazelcast_client &client);
            };

            class HAZELCAST_API true_predicate : public base_predicate {
            public:
                true_predicate(hazelcast_client &client);
            };

            class HAZELCAST_API instance_of_predicate : public base_predicate {
            public:
                /**
                 * @param javaClassName The name of the java class as identified by Class.get_name() in java.
                 */
                instance_of_predicate(hazelcast_client &client, const std::string &java_class_name);
            };

            class HAZELCAST_API sql_predicate : public base_predicate {
            public:
                /**
                 *
                 * @param client The client to be used for serialization.
                 * @param sql The sql query string.
                 */
                sql_predicate(hazelcast_client &client, const std::string &sql);
            };

            class HAZELCAST_API like_predicate : public named_predicate {
            public:
                /**
                 *
                 * @param attribute The name of the attribute
                 * @param expression The expression value to match
                 */
                like_predicate(hazelcast_client &client, const std::string &attribute, const std::string &expression);
            };

            class HAZELCAST_API ilike_predicate : public named_predicate {
            public:
                /**
                 *
                 * @param attribute The name of the attribute
                 * @param expression The expression value to match
                 */
                ilike_predicate(hazelcast_client &client, const std::string &attribute, const std::string &expression);
            };

            class HAZELCAST_API regex_predicate : public named_predicate {
            public:
                /**
                 *
                 * @param attribute The name of the attribute
                 * @param expression The expression value to match
                 */
                regex_predicate(hazelcast_client &client, const std::string &attribute, const std::string &expression);
            };

            class in_predicate : public multi_predicate {
            public:
                /**
                 * The type of Args should be able to be serialized.
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam value The values to search for
                 */
                template<typename ...Args>
                in_predicate(hazelcast_client &client, const std::string &attribute_name, const Args &...values)
                        : multi_predicate(attribute_name, client, values...) {}

                /**
                 * The type of Args should be able to be serialized.
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam value The values to search for
                 */
                template<typename T>
                in_predicate(hazelcast_client &client, const std::string &attribute_name, const std::vector<T> &values)
                        : multi_predicate(attribute_name, client, values) {}
            };

            class and_predicate : public multi_predicate {
            public:
                template<typename ...Args>
                and_predicate(hazelcast_client &client, const Args &...values) : multi_predicate(client, values...) {}
            };

            class or_predicate : public multi_predicate {
            public:
                template<typename ...PredicateTypes>
                or_predicate(hazelcast_client &client, const PredicateTypes &...values) : multi_predicate(client, values...) {}
            };

            class not_predicate : public base_predicate {
            public:
                template<typename T>
                not_predicate(hazelcast_client &client, const T &predicate) : base_predicate(client) {
                    out_stream.write_object(predicate);
                }
            };
        }

        namespace serialization {
            template<typename T>
            struct BasePredicateSerializer : public identified_data_serializer {
                /**
                 * @return factory id
                 */
                static constexpr int32_t get_factory_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::F_ID);
                }

                /**
                 * Defines how this class will be written.
                 * @param writer object_data_output
                 */
                static void write_data(const T &object, object_data_output &out) {
                    out.append_bytes(object.out_stream.to_byte_array());
                }

                /**
                 * Should not be called at the client side!
                 */
                static T read_data(object_data_input &in) {
                    // Not need to read at the client side
                    BOOST_THROW_EXCEPTION(exception::hazelcast_serialization("readData",
                                                                                       "Client should not need to use readdata method!!!"));
                }
            };

            template<>
            struct hz_serializer<query::between_predicate> : public BasePredicateSerializer<query::between_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::BETWEEN_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::equal_predicate> : public BasePredicateSerializer<query::equal_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::EQUAL_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::not_equal_predicate> : public BasePredicateSerializer<query::not_equal_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::NOTEQUAL_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::greater_less_predicate>
                    : public BasePredicateSerializer<query::greater_less_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::GREATERLESS_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::false_predicate> : public BasePredicateSerializer<query::false_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::FALSE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::true_predicate> : public BasePredicateSerializer<query::true_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::TRUE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::like_predicate> : public BasePredicateSerializer<query::like_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::LIKE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::instance_of_predicate>
                    : public BasePredicateSerializer<query::instance_of_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::INSTANCEOF_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::sql_predicate>
                    : public BasePredicateSerializer<query::sql_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::SQL_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::ilike_predicate> : public BasePredicateSerializer<query::ilike_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::ILIKE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::regex_predicate> : public BasePredicateSerializer<query::regex_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::REGEX_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::in_predicate> : public BasePredicateSerializer<query::in_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::IN_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::and_predicate> : public BasePredicateSerializer<query::and_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::AND_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::or_predicate> : public BasePredicateSerializer<query::or_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::OR_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::not_predicate> : public BasePredicateSerializer<query::not_predicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t get_class_id() noexcept {
                    return static_cast<int32_t>(query::predicate_data_serializer_hook::NOT_PREDICATE);
                }
            };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 
