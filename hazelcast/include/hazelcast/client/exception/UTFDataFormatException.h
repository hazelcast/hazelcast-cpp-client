/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
// Created by sancar koyunlu on 7/11/13.





#ifndef HAZELCAST_UTFDATAFORMATEXCEPTION
#define HAZELCAST_UTFDATAFORMATEXCEPTION

#include "hazelcast/client/exception/IException.h"

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Raised the string format is not correct according to UTF-8
             */
            class HAZELCAST_API UTFDataFormatException : public IException {
            public:
                /**
                 * Constructor
                 */
                UTFDataFormatException(const std::string &source, const std::string &message);

                /**
                 * Destructor
                 */
                ~UTFDataFormatException() throw();

                /**
                 * return exception explanation string.
                 */
                char const *what() const throw();
            };
        }
    }
}


#endif //HAZELCAST_UTFDATAFORMATEXCEPTION

