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

#include <iosfwd>

#include "hazelcast/util/export.h"
#include "hazelcast/util/byte.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
/**
 * A generic version to be used with VersionAware classes. The version is composed of two bytes, denoting
 * {@code major.minor} version. It is used to represent the Hazelcast cluster version and the serialization version of
 * VersionAware classes.
 *
 * @since 5.4
 */
struct HAZELCAST_API version
{
    // Accessible for testing
    /**
     * Use 0 as major &amp; minor values for UNKNOWN version
     */
    //static constexpr byte UNKNOWN_VERSION = 0;

    /**
     * Version 0.0 is UNKNOWN constant
     * <ul>
     * <li>{@code UNKNOWN} is only equal to itself.</li>
     * <li>{@code is(Less|Greater)Than} method with an {@code UNKNOWN} operand returns false.</li>
     * <li>{@code is(Less|Greater)OrEqual} with an {@code UNKNOWN} operand returns false, unless both operands are
     * {@code UNKNOWN}.</li>
     * <li>{@code UNKNOWN.isUnknown(Less|Greater)(Than|OrEqual)} returns true.</li>
     * <li>{@code otherVersion.isUnknown(Less|Greater)(Than|OrEqual)} with an {@code UNKNOWN} argument returns false.</li>
     * </ul>
     */


    byte major;
    byte minor;

/*
    bool operator==(const version& rhs) const;

    bool operator!=(const version& rhs) const;

    bool operator<(const version& rhs) const;

    bool operator>(const version& rhs) const;

    bool operator<=(const version& rhs) const;

    bool operator>=(const version& rhs) const;

    friend std::ostream HAZELCAST_API& operator<<(std::ostream& os,
                                                  const version& version);
*/
};

} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif