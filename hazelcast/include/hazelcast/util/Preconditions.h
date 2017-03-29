/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/util/SharedPtr.h"
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
             static int checkPositive(int value, const std::string &errorMessage);

            /**
             * Tests if an argument is not null.
             *
             * @param argument     the argument tested to see if it is not null.
             * @param errorMessage the errorMessage
             * @throws NullPointerException if argument is null
             */
            template <typename T>
            static hazelcast::util::SharedPtr<T> checkNotNull(const hazelcast::util::SharedPtr<T> &argument, const std::string &errorMessage) {
                if (argument.get() == NULL) {
                    throw client::exception::NullPointerException(errorMessage);
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
            static int checkNotNegative(int value, const std::string &errorMessage);
        };
    }
}


#endif //HAZELCAST_UTIL_PRECONDITIONS_H_

