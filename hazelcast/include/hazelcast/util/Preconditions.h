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
//
// Created by ihsan demir on 9 Dec 2016.

#ifndef HAZELCAST_UTIL_PRECONDITIONS_H_
#define HAZELCAST_UTIL_PRECONDITIONS_H_

#include <string>
#include <boost/shared_ptr.hpp>
#include "hazelcast/util/HazelcastDll.h"

#include "hazelcast/client/exception/ProtocolExceptions.h"

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
             * @throws IllegalArgumentException if the value is not positive.
             */
            template <typename T>
            static const T &checkPositive(const T &value, const std::string &errorMessage) {
                if (value <= 0) {
                    throw client::exception::IllegalArgumentException("Preconditions::checkPositive", errorMessage);
                }
                return value;
            }

            /**
             * Tests if an argument is not null.
             *
             * @param argument     the argument tested to see if it is not null.
             * @param errorMessage the errorMessage
             * @throws NullPointerException if argument is null
             */
            template<typename T>
            static const boost::shared_ptr<T> &checkNotNull(const boost::shared_ptr<T> &argument,
                                                     const std::string &errorMessage) {
                if (argument == NULL) {
                    throw client::exception::NullPointerException(errorMessage);
                }
                return argument;
            }

            /**
             * Tests if an argument is not null.
             *
             * @param argument the argument tested to see if it is not null.
             * @param argName  the string name (used in message if an error is thrown).
             * @return the string argument that was tested.
             * @throws IllegalArgumentException if the argument is null.
             */
            template<typename T>
            static const boost::shared_ptr<T> &isNotNull(const boost::shared_ptr<T> &argument,
                                                     const std::string argName) {
                if (argument == NULL) {
                    throw (client::exception::ExceptionBuilder<client::exception::IllegalArgumentException>("")
                            << "argument " << argName << " can't be null").build();
                }
                return argument;
            }

            /**
             * Tests if a value is not negative.
             *
             * @param value        the  value tested to see if it is not negative.
             * @param errorMessage the errorMessage
             * @return the value
             * @throws java.lang.IllegalArgumentException if the value is negative.
             */
            template <typename T>
            static const T &checkNotNegative(const T &value, const std::string &errorMessage) {
                if (value < 0) {
                    throw client::exception::IllegalArgumentException(errorMessage);
                }
                return value;
            }

            /**
             * Tests if a string contains text.
             *
             * @param argument     the string tested to see if it contains text.
             * @param source the source where the check is performed
             * @param errorMessage the errorMessage
             * @return the string argument that was tested.
             * @throws client::exception::IllegalArgumentException if the string is empty
             */
            static const std::string &checkHasText(const std::string &argument, const std::string &errorMessage);

            /**
             * @throws client::exception::InvalidConfigurationException if the user does not compile with
             * HZ_BUILD_WITH_SSL flag but is trying to use a feature (e.g. TLS, AWS Cloud Discovery) that needs this flag.
             */
            static void checkSSL(const std::string &sourceMethod);

            /**
             * Tests whether the supplied expression is {@code true}.
             *
             * @param expression   the expression tested to see if it is {@code true}.
             * @param errorMessage the errorMessage
             * @throws client::Exception::IllegalArgumentException if the supplied expression is {@code false}.
             */
            static void checkTrue(bool expression, const std::string &errorMessage);
        };
    }
}


#endif //HAZELCAST_UTIL_PRECONDITIONS_H_

