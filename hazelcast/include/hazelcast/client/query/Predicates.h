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

#include "hazelcast/client/serialization/serialization.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        class HazelcastClient;

        namespace query {
            struct HAZELCAST_API QueryConstants {
                static constexpr const char *KEY_ATTRIBUTE_NAME = "__key";
                static constexpr const char *THIS_ATTRIBUTE_NAME = "this";
            };

            /**
             * This is a marker class for Predicate classes. All predicate classes shall extend this class. This class
             * also indicates that a Java implementation is required at the server side.
             */
            class HAZELCAST_API Predicate {
            };

            struct HAZELCAST_API BasePredicate : public Predicate {
                explicit BasePredicate(HazelcastClient &client);

                serialization::ObjectDataOutput outStream;
            };

            class HAZELCAST_API NamedPredicate : public BasePredicate {
            protected:
                explicit NamedPredicate(HazelcastClient &client, const std::string &attributeName);
            };

            class MultiPredicate : public BasePredicate {
            public:
                template<typename ...Args>
                MultiPredicate(HazelcastClient &client, const Args &...values) : BasePredicate(client) {
                    outStream.write<int32_t>(static_cast<int32_t>(sizeof...(values)));
                    outStream.writeObjects(values...);
                }

                template<typename ...Args>
                MultiPredicate(const std::string attributeName, HazelcastClient &client, const Args &...values) : BasePredicate(client) {
                    outStream.write(attributeName);
                    outStream.write<int32_t>(static_cast<int32_t>(sizeof...(values)));
                    outStream.writeObjects(values...);
                }
            };

            enum struct PredicateDataSerializerHook {
                F_ID = -32,

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

            class EqualPredicate : public NamedPredicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam from The value of the attribute.
                 */
                template<typename T>
                EqualPredicate(HazelcastClient &client, const std::string &attributeName, const T &value)
                        : NamedPredicate(client, attributeName) {
                    outStream.writeObject(value);
                }
            };

            class NotEqualPredicate : public NamedPredicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam from The value of the attribute.
                 */
                template<typename T>
                NotEqualPredicate(HazelcastClient &client, const std::string &attributeName, const T &value)
                        : NamedPredicate(client, attributeName) {
                    outStream.writeObject(value);
                }
            };

            class GreaterLessPredicate : public NamedPredicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam value The value to compare
                 * @param equal Allow equal matching
                 * @param less If true, allow "less than" matching otherwise do "greater than" matching
                 */
                template<typename T>
                GreaterLessPredicate(HazelcastClient &client, const std::string &attributeName, const T &value,
                                     bool isEqual, bool isLess)
                        : NamedPredicate(client, attributeName) {
                    outStream.writeObject(value);
                    outStream.write(isEqual);
                    outStream.write(isLess);
                }
            };

            class BetweenPredicate : public NamedPredicate {
            public:
                /**
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam from The starting value to match (start is inclusive).
                 * @tparam to The ending value to match (end is inclusive).
                 */
                template<typename FROM_TYPE, typename TO_TYPE>
                BetweenPredicate(HazelcastClient &client, const std::string &attributeName, const FROM_TYPE &from,
                                 const TO_TYPE &to)
                        : NamedPredicate(client, attributeName) {
                    outStream.writeObject(to);
                    outStream.writeObject(from);
                }
            };

            class HAZELCAST_API FalsePredicate : public BasePredicate {
            public:
                FalsePredicate(HazelcastClient &client);
            };

            class HAZELCAST_API TruePredicate : public BasePredicate {
            public:
                TruePredicate(HazelcastClient &client);
            };

            class HAZELCAST_API InstanceOfPredicate : public BasePredicate {
            public:
                /**
                 * @param javaClassName The name of the java class as identified by Class.getName() in java.
                 */
                InstanceOfPredicate(HazelcastClient &client, const std::string &javaClassName);
            };

            class HAZELCAST_API SqlPredicate : public BasePredicate {
            public:
                /**
                 *
                 * @param client The client to be used for serialization.
                 * @param sql The sql query string.
                 */
                SqlPredicate(HazelcastClient &client, const std::string &sql);
            };

            class HAZELCAST_API LikePredicate : public NamedPredicate {
            public:
                /**
                 *
                 * @param attribute The name of the attribute
                 * @param expression The expression value to match
                 */
                LikePredicate(HazelcastClient &client, const std::string &attribute, const std::string &expression);
            };

            class HAZELCAST_API ILikePredicate : public NamedPredicate {
            public:
                /**
                 *
                 * @param attribute The name of the attribute
                 * @param expression The expression value to match
                 */
                ILikePredicate(HazelcastClient &client, const std::string &attribute, const std::string &expression);
            };

            class HAZELCAST_API RegexPredicate : public NamedPredicate {
            public:
                /**
                 *
                 * @param attribute The name of the attribute
                 * @param expression The expression value to match
                 */
                RegexPredicate(HazelcastClient &client, const std::string &attribute, const std::string &expression);
            };

            class InPredicate : public MultiPredicate {
            public:
                /**
                 * The type of Args should be able to be serialized.
                 * @param attributeName The attribute whose value shall be compared to.
                 * @tparam value The values to search for
                 */
                template<typename ...Args>
                InPredicate(HazelcastClient &client, const std::string &attributeName, const Args &...values)
                        : MultiPredicate(attributeName, client, values...) {}
            };

            class AndPredicate : public MultiPredicate {
            public:
                template<typename ...Args>
                AndPredicate(HazelcastClient &client, const Args &...values) : MultiPredicate(client, values...) {}
            };

            class OrPredicate : public MultiPredicate {
            public:
                template<typename ...PredicateTypes>
                OrPredicate(HazelcastClient &client, const PredicateTypes &...values) : MultiPredicate(client, values...) {}
            };

            class NotPredicate : public BasePredicate {
            public:
                template<typename T>
                NotPredicate(HazelcastClient &client, const T &predicate) : BasePredicate(client) {
                    outStream.writeObject(predicate);
                }
            };
        }

        namespace serialization {
            template<typename T>
            struct BasePredicateSerializer : public identified_data_serializer {
                /**
                 * @return factory id
                 */
                static constexpr int32_t getFactoryId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::F_ID);
                }

                /**
                 * Defines how this class will be written.
                 * @param writer ObjectDataOutput
                 */
                static void writeData(const T &object, ObjectDataOutput &out) {
                    out.appendBytes(object.outStream.toByteArray());
                }

                /**
                 * Should not be called at the client side!
                 */
                static T readData(ObjectDataInput &in) {
                    // Not need to read at the client side
                    BOOST_THROW_EXCEPTION(exception::HazelcastSerializationException("readData",
                                                                                     "Client should not need to use readData method!!!"));
                }
            };

            template<>
            struct hz_serializer<query::BetweenPredicate> : public BasePredicateSerializer<query::BetweenPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::BETWEEN_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::EqualPredicate> : public BasePredicateSerializer<query::EqualPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::EQUAL_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::NotEqualPredicate> : public BasePredicateSerializer<query::NotEqualPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::NOTEQUAL_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::GreaterLessPredicate>
                    : public BasePredicateSerializer<query::GreaterLessPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::GREATERLESS_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::FalsePredicate> : public BasePredicateSerializer<query::FalsePredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::FALSE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::TruePredicate> : public BasePredicateSerializer<query::TruePredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::TRUE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::LikePredicate> : public BasePredicateSerializer<query::LikePredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::LIKE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::InstanceOfPredicate>
                    : public BasePredicateSerializer<query::InstanceOfPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::INSTANCEOF_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::SqlPredicate>
                    : public BasePredicateSerializer<query::SqlPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::SQL_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::ILikePredicate> : public BasePredicateSerializer<query::ILikePredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::ILIKE_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::RegexPredicate> : public BasePredicateSerializer<query::RegexPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::REGEX_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::InPredicate> : public BasePredicateSerializer<query::InPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::IN_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::AndPredicate> : public BasePredicateSerializer<query::AndPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::AND_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::OrPredicate> : public BasePredicateSerializer<query::OrPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::OR_PREDICATE);
                }
            };

            template<>
            struct hz_serializer<query::NotPredicate> : public BasePredicateSerializer<query::NotPredicate> {
                /**
                 * @return class id
                 */
                static constexpr int32_t getClassId() noexcept {
                    return static_cast<int32_t>(query::PredicateDataSerializerHook::NOT_PREDICATE);
                }
            };
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 
