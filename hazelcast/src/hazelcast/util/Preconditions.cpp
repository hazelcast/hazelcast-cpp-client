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

#include "hazelcast/util/Preconditions.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace util {
        int Preconditions::checkPositive(int value, const std::string &errorMessage) {
            if (value <= 0) {
                throw client::exception::IllegalArgumentException("Preconditions::checkPositive", errorMessage);
            }
            return value;
        }

        int Preconditions::checkNotNegative(int value, const std::string &errorMessage) {
            if (value < 0) {
                throw client::exception::IllegalArgumentException(errorMessage);
            }
            return value;
        }

        const std::string &Preconditions::checkHasText(const std::string &argument,
                                                       const std::string &errorMessage) {
            if (argument.empty()) {
                throw client::exception::IllegalArgumentException("", errorMessage);
            }

            return argument;
        }

        void Preconditions::checkSSL(const std::string &sourceMethod) {
            #ifndef HZ_BUILD_WITH_SSL
            throw client::exception::InvalidConfigurationException(sourceMethod, "You should compile with "
                    "HZ_BUILD_WITH_SSL flag. You should also have the openssl installed on your machine and you need "
                    "to link with the openssl library.");
            #endif
        }
    }
}


