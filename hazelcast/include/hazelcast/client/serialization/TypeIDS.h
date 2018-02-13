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

#ifndef HAZELCAST_TypeIDS
#define HAZELCAST_TypeIDS

#include <string>
#include <vector>
#include <stdint.h>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Portable;
            class IdentifiedDataSerializable;

            int32_t HAZELCAST_API getHazelcastTypeId(const Portable* portable);

            int32_t HAZELCAST_API getHazelcastTypeId(const IdentifiedDataSerializable* identifiedDataSerializable);

            int32_t HAZELCAST_API getHazelcastTypeId(const char *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const bool *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const byte *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const int16_t *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const int32_t *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const int64_t *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const float *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const double *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::string *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<char> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<bool> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<byte> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<int16_t> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<int32_t> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<int64_t> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<float> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<double> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<std::string *> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const std::vector<std::string> *object);

            int32_t HAZELCAST_API getHazelcastTypeId(const void *object);
        }
    }
}

#endif //HAZELCAST_TypeIDS
