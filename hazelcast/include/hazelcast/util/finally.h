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

#include <functional>
#include "hazelcast/util/export.h"

namespace hazelcast {
namespace util {
class HAZELCAST_API finally
{
    std::function<void(void)> functor;

public:
    finally(const std::function<void(void)>& functor)
      : functor(functor)
    {}
    ~finally() { functor(); }
};
} // namespace util
} // namespace hazelcast