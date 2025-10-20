/*
* Copyright (c) 2008-2025, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/util/byte.h"

namespace hazelcast {
namespace client {
namespace internal {
/**
 * A generic version to be used with version aware classes. The version is composed of two bytes, denoting
 * {@code MAJOR.MINOR} version. It is used to represent the Hazelcast cluster version.
 *
 * @since 5.5
 */
struct version
{
    byte major;
    byte minor;
};
} // namespace impl
} // namespace client
} // namespace hazelcast