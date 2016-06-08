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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CLIENTEXCEPTIONFACTORY_H_
#define HAZELCAST_CLIENT_PROTOCOL_CLIENTEXCEPTIONFACTORY_H_

#include "hazelcast/util/HazelcastDll.h"
#include <memory>
#include <map>
#include <stdint.h>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace exception {
            class IException;
        }

        namespace protocol {
            class ClientMessage;

            class ExceptionFactory {
            public:
                virtual ~ExceptionFactory();

                virtual std::auto_ptr<exception::IException> createException(const std::string &message,
                                                                             const std::string &details,
                                                                             int32_t errorCode,
                                                                             int32_t causeErrorCode) = 0;
            };

            class HAZELCAST_API ClientExceptionFactory {
            public:
                ClientExceptionFactory();

                virtual ~ClientExceptionFactory();

                std::auto_ptr<exception::IException> createException(protocol::ClientMessage &message) const;
            private:
                void registerException(int32_t errorCode, ExceptionFactory *factory);

                std::map<int32_t, ExceptionFactory *> errorCodeToFactory;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HAZELCAST_CLIENT_PROTOCOL_CLIENTEXCEPTIONFACTORY_H_
