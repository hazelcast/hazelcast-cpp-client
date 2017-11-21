/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//

#include "hazelcast/client/serialization/TypeIDS.h"
#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            int32_t getHazelcastTypeId(const Portable* portable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            }

            int32_t getHazelcastTypeId(const IdentifiedDataSerializable* identifiedDataSerializable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int32_t getHazelcastTypeId(const char *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR;
            }

            int32_t getHazelcastTypeId(const bool *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            }

            int32_t getHazelcastTypeId(const byte *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE;
            }

            int32_t getHazelcastTypeId(const int16_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT;
            }

            int32_t getHazelcastTypeId(const int32_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER;
            }

            int32_t getHazelcastTypeId(const int64_t *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_LONG;
            }

            int32_t getHazelcastTypeId(const float *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT;
            }

            int32_t getHazelcastTypeId(const double *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE;
            }

            int32_t getHazelcastTypeId(const std::string *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING;
            }

            int32_t getHazelcastTypeId(const std::vector<char> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<bool> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<byte> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int16_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int32_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<int64_t> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<float> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<double> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<std::string *> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
            }

            int32_t getHazelcastTypeId(const std::vector<std::string> *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING_ARRAY;
            }
        }
    }
}

