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

#if defined(HZ_HAS_CXX17_SUPPORT)
#include <optional>
#else
#include <boost/optional.hpp>
#endif

namespace hazelcast {
namespace util {
    
#if defined(HZ_HAS_CXX17_SUPPORT)
    template <typename T> using optional = std::optional<T>;
    template <typename... T>
    auto make_optional(T&&... t) -> decltype(std::make_optional(std::forward<T>(t)...))
    {
        return std::make_optional(std::forward<T>(t)...);
    }
#else
    template <typename T> using optional = boost::optional<T>;
    template <typename... T>
    auto make_optional(T&&... t) -> decltype(boost::make_optional(std::forward<T>(t)...))
    {
        return boost::make_optional(std::forward<T>(t)...);
    }
#endif
} // namespace client
} // namespace hazelcast