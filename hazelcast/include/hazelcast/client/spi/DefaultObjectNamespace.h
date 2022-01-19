/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace spi {
/**
 * An ObjectNamespace that makes identification of object within a service
 * possible.
 */
class HAZELCAST_API DefaultObjectNamespace
{
public:
    DefaultObjectNamespace(const std::string& service,
                           const std::string& object);

    /**
     * Gets the service name.
     *
     * @return the service name
     */
    const std::string& get_service_name() const;

    /**
     * Gets the object name within the service.
     *
     * @return the object name within the service
     */
    const std::string& get_object_name() const;

    bool operator==(const DefaultObjectNamespace& rhs) const;

private:
    std::string service_name_;
    std::string object_name_;
};
} // namespace spi
} // namespace client
} // namespace hazelcast

namespace std {
template<>
struct less<hazelcast::client::spi::DefaultObjectNamespace>
{
    bool operator()(
      const hazelcast::client::spi::DefaultObjectNamespace& lhs,
      const hazelcast::client::spi::DefaultObjectNamespace& rhs) const;
};

template<>
struct HAZELCAST_API hash<hazelcast::client::spi::DefaultObjectNamespace>
{
    std::size_t operator()(
      const hazelcast::client::spi::DefaultObjectNamespace& k) const noexcept;
};
} // namespace std

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
