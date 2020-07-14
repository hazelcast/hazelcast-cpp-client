/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include <assert.h>
#include <memory>

#include "hazelcast/util/Iterator.h"

namespace hazelcast {
    namespace util {
        template <typename T>
        class Iterable {
        public:
            virtual ~Iterable() = default;

            /**
             * Returns an iterator over a set of elements of type T.
             *
             * @return an Iterator.
             */
            virtual Iterator<T> *iterator() {
                assert(0);
                return NULL;
            }
        };
    }
}



