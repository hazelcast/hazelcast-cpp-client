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
#include <sstream>

#include "hazelcast/util/Closeable.h"
#include "hazelcast/client/exception/iexception.h"

namespace hazelcast {
namespace util {
class HAZELCAST_API IOUtil
{
public:
    template<typename T>
    static std::string to_string(T value)
    {
        std::stringstream s;
        s << value;
        return s.str();
    }

    template<typename T>
    static T to_value(const std::string& str)
    {
        std::stringstream s(str);
        T value;
        s >> value;
        return value;
    }

    static void close_resource(Closeable* closable,
                               const char* close_reason = nullptr);
};

template<>
bool
IOUtil::to_value(const std::string& str);

} // namespace util
} // namespace hazelcast
