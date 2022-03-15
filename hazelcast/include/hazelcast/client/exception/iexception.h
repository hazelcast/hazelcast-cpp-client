/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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
#include <stdexcept>
#include <ostream>

#include <boost/format.hpp>
#include <boost/exception_ptr.hpp>

#include "hazelcast/util/export.h"

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
             * @see interrupted_exception
             * @see io_exception
             * @see hazelcast_serialization_exception
             * @see Iclass_cast_exception
             * @see illegal_state_exception
             * @see illegal_argument_exception
             */
            class HAZELCAST_API iexception : public std::exception {
            public:
                iexception();

                // TODO: Remove isRuntime and retryable and use the derived class concept as in Java
                iexception(const std::string &exception_name, const std::string &source, const std::string &message,
                           const std::string &details, int32_t error_no, std::exception_ptr cause, bool is_runtime,
                           bool retryable);

                ~iexception() noexcept override;

                /**
                 *
                 * return  pointer to the explanation string.
                 */
                char const *what() const noexcept override;

                const std::string &get_source() const;

                const std::string &get_message() const;

                const std::string &get_details() const;

                int32_t get_error_code() const;

                bool is_runtime() const;

                bool is_retryable() const;

                friend std::ostream HAZELCAST_API &operator<<(std::ostream &os, const iexception &exception);

            protected:
                std::string src_;
                std::string msg_;
                std::string details_;
                int32_t error_code_;
                std::exception_ptr cause_;
                bool runtime_exception_;
                bool retryable_;
                std::string report_;
            };

            std::ostream HAZELCAST_API &operator<<(std::ostream &os, const iexception &exception);

            template<typename EXCEPTIONCLASS>
            class exception_builder {
            public:
                explicit exception_builder(const std::string &source) : source_(source) {}

                template<typename T>
                exception_builder &operator<<(const T &message) {
                    msg_ << message;
                    return *this;
                }

                /**
                 *
                 * @return The constructed exception.
                 */
                boost::exception_detail::clone_impl<EXCEPTIONCLASS> build() {
                    return boost::enable_current_exception(EXCEPTIONCLASS(source_, msg_.str()));
                }
            private:
                std::string source_;
                std::ostringstream msg_;
            };

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


