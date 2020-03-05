/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_EXCEPTION
#define HAZELCAST_EXCEPTION

#include <string>
#include <sstream>
#include <stdexcept>
#include <ostream>
#include <memory>

#include <memory>

#include "hazelcast/util/HazelcastDll.h"

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
                IException(const std::string &exceptionName, const std::string &source, const std::string &message,
                           const std::string &details, int32_t errorNo, int32_t causeCode, bool isRuntime, bool retryable = false);

                IException(const std::string &exceptionName, const std::string &source, const std::string &message,
                           int32_t errorNo, int32_t causeCode, bool isRuntime, bool retryable = false);

                IException(const std::string &exceptionName, const std::string &source, const std::string &message,
                           int32_t errorNo, bool isRuntime, bool retryable = false);

                IException(const std::string &exceptionName, const std::string &source, const std::string &message,
                           int32_t errorNo, const std::shared_ptr<IException> &cause, bool isRuntime, bool retryable = false);

                virtual ~IException() throw();

                /**
                 *
                 * return  pointer to the explanation string.
                 */
                virtual char const *what() const throw();

                const std::string &getSource() const;

                const std::string &getMessage() const;

                virtual void raise() const;

                /**
                 * We need this method to clone the specific derived exception when needed. The exception has to be the
                 * derived type so that the exception rethrowing works as expected by throwing the derived exception.
                 * Exception throwing internals works by making a temporary copy of the exception.
                 * @return The copy of this exception
                 */
                virtual std::unique_ptr<IException> clone() const;

                const std::string &getDetails() const;

                int32_t getErrorCode() const;

                int32_t getCauseErrorCode() const;

                const std::shared_ptr<IException> &getCause() const;

                bool isRuntimeException() const;

                bool isRetryable() const;

                friend std::ostream HAZELCAST_API &operator<<(std::ostream &os, const IException &exception);

            protected:
                std::string src;
                std::string msg;
                std::string details;
                std::string report;
                int32_t errorCode;
                int32_t causeErrorCode;
                std::shared_ptr<IException> cause;
                bool runtimeException;
                bool retryable;
            };

            std::ostream HAZELCAST_API &operator<<(std::ostream &os, const IException &exception);

            template<typename EXCEPTIONCLASS>
            class ExceptionBuilder {
            public:
                ExceptionBuilder(const std::string &source) : source(source) {}

                template<typename T>
                ExceptionBuilder &operator<<(const T &message) {
                    msg << message;
                    return *this;
                }

                /**
                 *
                 * @return The constructed exception.
                 */
                EXCEPTIONCLASS build() {
                    return EXCEPTIONCLASS(source, msg.str());
                }

                std::shared_ptr<EXCEPTIONCLASS> buildShared() {
                    return std::shared_ptr<EXCEPTIONCLASS>(new EXCEPTIONCLASS(source, msg.str()));
                }

            private:
                std::string source;
                std::ostringstream msg;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_EXCEPTION
