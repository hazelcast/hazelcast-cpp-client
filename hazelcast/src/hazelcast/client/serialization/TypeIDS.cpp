/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
            int getHazelcastTypeId(const Portable* portable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_PORTABLE;
            }

            int getHazelcastTypeId(const IdentifiedDataSerializable* identifiedDataSerializable) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DATA;
            }

            int getHazelcastTypeId(const char *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_CHAR;
            }

            int getHazelcastTypeId(const bool *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            }

            int getHazelcastTypeId(const byte *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_BYTE;
            }

            int getHazelcastTypeId(const short *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_SHORT;
            }

            int getHazelcastTypeId(const int *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_INTEGER;
            }

            int getHazelcastTypeId(const float *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_FLOAT;
            }

            int getHazelcastTypeId(const double *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_DOUBLE;
            }

            int getHazelcastTypeId(const std::string *object) {
                return pimpl::SerializationConstants::CONSTANT_TYPE_STRING;
            }
        }
    }
}

