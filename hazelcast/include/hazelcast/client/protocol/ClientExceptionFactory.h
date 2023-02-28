/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/export.h"
#include <unordered_map>
#include <string>
#include <vector>

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace exception {
class iexception;
}

namespace protocol {
namespace codec {
struct ErrorHolder;
}
class ClientMessage;

class ExceptionFactory;

class HAZELCAST_API ClientExceptionFactory
{
public:
    ClientExceptionFactory();

    virtual ~ClientExceptionFactory();

    std::exception_ptr create_exception(
      const std::vector<codec::ErrorHolder>& errors) const;

private:
    void register_exception(int32_t error_code, ExceptionFactory* factory);

    std::exception_ptr create_exception(
      std::vector<codec::ErrorHolder>::const_iterator begin,
      std::vector<codec::ErrorHolder>::const_iterator end) const;

    std::unordered_map<int32_t, ExceptionFactory*> error_code_to_factory_;
};

class HAZELCAST_API ExceptionFactory
{
public:
    virtual ~ExceptionFactory();

    virtual std::exception_ptr create_exception(
      const ClientExceptionFactory& factory,
      const std::string& source,
      const std::string& message,
      const std::string& details,
      std::exception_ptr cause) const = 0;
};

template<typename EXCEPTION>
class ExceptionFactoryImpl : public ExceptionFactory
{
public:
    std::exception_ptr create_exception(const ClientExceptionFactory& factory,
                                        const std::string& source,
                                        const std::string& message,
                                        const std::string& details,
                                        std::exception_ptr cause) const override
    {
        try {
            BOOST_THROW_EXCEPTION(EXCEPTION(source, message, details, cause));
        } catch (...) {
            return std::current_exception();
        }
    }
};

} // namespace protocol
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
