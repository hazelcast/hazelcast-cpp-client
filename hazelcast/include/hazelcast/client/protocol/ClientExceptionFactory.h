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

#include "hazelcast/util/HazelcastDll.h"
#include <memory>
#include <unordered_map>
#include <stdint.h>
#include <string>

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

            class ExceptionFactory;

            class HAZELCAST_API ClientExceptionFactory {
            public:
                ClientExceptionFactory();

                virtual ~ClientExceptionFactory();

                void throwException(const std::string &source,
                                    protocol::ClientMessage &clientMessage) const;

                void throwException(int32_t errorCode) const;

            private:
                void registerException(int32_t errorCode, ExceptionFactory *factory);

                std::unordered_map<int32_t, ExceptionFactory *> errorCodeToFactory;
            };

            class HAZELCAST_API ExceptionFactory {
            public:
                virtual ~ExceptionFactory();

                virtual void
                throwException(const ClientExceptionFactory &clientExceptionFactory, const std::string &source,
                               const std::string &message, const std::string &details,
                               int32_t causeErrorCode) const = 0;

                virtual void throwException() const = 0;
            };

            template<typename EXCEPTION>
            class ExceptionFactoryImpl : public ExceptionFactory {
            public:
                void throwException(const ClientExceptionFactory &clientExceptionFactory, const std::string &source,
                                    const std::string &message,
                                    const std::string &details = nullptr, int32_t causeErrorCode = -1) const {
                    EXCEPTION e(source, message, details);
                    if (causeErrorCode < 0) {
                        throw boost::enable_current_exception(e);
                    }

                    try {
                        clientExceptionFactory.throwException(causeErrorCode);
                    } catch (...) {
                        std::throw_with_nested(boost::enable_current_exception(e));
                    }
                }

                void throwException() const {
                    return BOOST_THROW_EXCEPTION(EXCEPTION());
                }
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


