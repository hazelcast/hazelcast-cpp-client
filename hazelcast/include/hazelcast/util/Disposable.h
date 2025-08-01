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

#include "hazelcast/util/export.h"

namespace hazelcast {
namespace util {
/**
 * A {@code Disposable} is a container of data and/or resources that can be
 * disposed.
 * {@link #dispose()} method is called to dispose contained data and/or
 * resources.
 */
class HAZELCAST_API Disposable
{
public:
    virtual ~Disposable() = default;

    /**
     * Disposes this object and releases any data and/or resources associated
     * with it. If this object is already disposed then invoking this
     * method has no effect.
     */
    virtual void dispose() = 0;
};
} // namespace util
} // namespace hazelcast
