/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include <memory>
#include <boost/uuid/uuid.hpp>
#include "hazelcast/util/export.h"

#include "hazelcast/client/exception/protocol_exceptions.h"

namespace hazelcast {
    namespace util {
        class HAZELCAST_API Preconditions {
        public:
            /**
             * Tests if a value is positive; larger than 0.
             *
             * @param value        the value tested to see if it is positive.
             * @param errorMessage the message
             * @return the value
             * @throws illegal_argument if the value is not positive.
             */
            template<typename T>
            static const T &check_positive(const T &value, const std::string &error_message) {
                if (value <= 0) {
                    throw client::exception::illegal_argument("Preconditions::checkPositive", error_message);
                }
                return value;
            }

            /**
             * Tests if an argument is not null.
             *
             * @param argument     the argument tested to see if it is not null.
             * @param errorMessage the errorMessage
             * @throws null_pointer if argument is null
             */
            template<typename T>
            static const std::shared_ptr<T> &check_not_null(const std::shared_ptr<T> &argument,
                                                          const std::string &error_message) {
                check_not_null<T>(argument.get(), error_message);
                return argument;
            }

            template<typename T>
            static const T *check_not_null(const T *argument, const std::string &error_message) {
                if (!argument) {
                    throw client::exception::null_pointer(error_message);
                }
                return argument;
            }

            static void check_not_nill(boost::uuids::uuid id, const std::string &error_message) {
                if (id.is_nil()) {
                    throw client::exception::null_pointer(error_message);
                }
            }

            /**
             * Tests whether the supplied expression is {@code true}.
             *
             * @param expression   the expression tested to see if it is {@code true}.
             * @param errorMessage the errorMessage
             * @throws illegal_argument if the supplied expression is {@code false}.
             */
            template<typename T>
            static void check_true(const T &argument, const std::string &error_message) {
                if (!argument) {
                    throw client::exception::illegal_argument(error_message);
                }
            }

            /**
             * Tests if an argument is not empty. The argument should have the size method.
             *
             * @param argument     the argument tested to see if it is not empty.
             * @param errorMessage the errorMessage
             * @throws illegal_argument if argument is empty
             */
            template<typename T>
            static void check_not_empty(const T &argument, const std::string &error_message) {
                if (argument.size() == 0) {
                    throw client::exception::illegal_argument(error_message);
                }
            }

            /**
             * Tests if an argument is not null.
             *
             * @param argument the argument tested to see if it is not null.
             * @param argName  the string name (used in message if an error is thrown).
             * @return the string argument that was tested.
             * @throws illegal_argument if the argument is null.
             */
            template<typename T>
            static const std::shared_ptr<T> &is_not_null(const std::shared_ptr<T> &argument,
                                                         const std::string &arg_name) {
                if (argument == NULL) {
                    throw (client::exception::exception_builder<client::exception::illegal_argument>("")
                            << "argument " << arg_name << " can't be null").build();
                }
                return argument;
            }

            /**
             * Tests if a value is not negative.
             *
             * @param value        the  value tested to see if it is not negative.
             * @param errorMessage the errorMessage
             * @return the value
             * @throws illegal_argument if the value is negative.
             */
            template<typename T>
            static const T &check_not_negative(const T &value, const std::string &error_message) {
                if (value < 0) {
                    throw client::exception::illegal_argument("Preconditions::checkNotNegative", error_message);
                }
                return value;
            }

            /**
             * Tests if a value is less than a maximum allowed value.
             *
             * @param actualSize        the  value tested to see if it is not negative.
             * @param expectedMaximum   the maximum allowed
             * @param variableName      The neame of the variable to be printed in the exception message
             * @throws illegal_argument if the value is greater than expectedMaximum.
             */
            template<typename T>
            static void check_max(const T &actual_size, const T &expected_maximum, const std::string &variable_name) {
                if (actual_size > expected_maximum) {
                    throw (client::exception::exception_builder<client::exception::illegal_argument>(
                            "Preconditions::check_max") << variable_name << "(" << actual_size << ")"
                                                        << " can't be larger than " << expected_maximum).build();
                }
            }

            /**
             * Tests if a string contains text.
             *
             * @param argument     the string tested to see if it contains text.
             * @param source the source where the check is performed
             * @param errorMessage the errorMessage
             * @return the string argument that was tested.
             * @throws client::exception::illegal_argument if the string is empty
             */
            static const std::string &check_has_text(const std::string &argument, const std::string &error_message);

            /**
             * @throws client::exception::invalid_configuration if the user does not compile with
             * HZ_BUILD_WITH_SSL flag but is trying to use a feature (e.g. TLS, AWS Cloud Discovery) that needs this flag.
             */
            static void check_ssl(const std::string &source_method);

            /**
             * Tests whether the supplied expression is {@code true}.
             *
             * @param expression   the expression tested to see if it is {@code true}.
             * @param errorMessage the errorMessage
             * @throws client::Exception::illegal_argument if the supplied expression is {@code false}.
             */
            static void check_true(bool expression, const std::string &error_message);
        };
    }
}




