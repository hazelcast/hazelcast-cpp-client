/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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

#ifndef HAZELCAST_CLIENT_IMPL_IDGENERATORINTERFACE_H_
#define HAZELCAST_CLIENT_IMPL_IDGENERATORINTERFACE_H_

#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class HAZELCAST_API IdGeneratorInterface {
            public:
                 virtual int64_t newId() = 0;

                 virtual bool init(int64_t id) = 0;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_IDGENERATORINTERFACE_H_

