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
// Created by msk on 3/13/13.
//
// To change the template use AppCode | Preferences | File Templates.
//
#ifndef HAZELCAST_EXCEPTION
#define HAZELCAST_EXCEPTION

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <stdexcept>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#pragma warning(disable: 4275) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        namespace exception {
            /**
             * Base class for all exception originated from Hazelcast methods.
             * If exception coming from hazelcast servers cannot be identified,
             * it will be fired as IException.
             *
             *
             * @see InstanceNotActiveException
             * @see InterruptedException
             * @see IOException
             * @see HazelcastSerializationException
             * @see IClassCastException
             * @see IllegalStateException
             * @see IllegalArgumentException
             */
            class HAZELCAST_API IException : public std::exception {
            public:
                /**
                 * Constructor
                 */
                IException();

                /**
                 * Constructor
                 */
                IException(const std::string &source, const std::string &message);

                /**
                 * Destructor
                 */
                virtual ~IException() throw();

                /**
                 * return exception explanation string.
                 */
                virtual char const *what() const throw();

                const std::string &getSource() const;

                const std::string &getMessage() const;

                virtual void raise();
            private:
                std::string src;
                std::string msg;
                std::string report;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif //HAZELCAST_EXCEPTION


