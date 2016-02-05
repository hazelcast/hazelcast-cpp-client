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
// Created by sancar koyunlu on 21/02/14.
//

#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/IClassCastException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                int const SerializationConstants::CONSTANT_TYPE_NULL = 0;
                int const SerializationConstants::CONSTANT_TYPE_PORTABLE=-1;
                int const SerializationConstants::CONSTANT_TYPE_DATA=-2;
                int const SerializationConstants::CONSTANT_TYPE_BYTE=-3;
                int const SerializationConstants::CONSTANT_TYPE_BOOLEAN=-4;
                int const SerializationConstants::CONSTANT_TYPE_CHAR=-5;
                int const SerializationConstants::CONSTANT_TYPE_SHORT=-6;
                int const SerializationConstants::CONSTANT_TYPE_INTEGER=-7;
                int const SerializationConstants::CONSTANT_TYPE_LONG=-8;
                int const SerializationConstants::CONSTANT_TYPE_FLOAT=-9;
                int const SerializationConstants::CONSTANT_TYPE_DOUBLE=-10;
                int const SerializationConstants::CONSTANT_TYPE_STRING=-11;
                int const SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY=-12;
                int const SerializationConstants::CONSTANT_TYPE_BOOLEAN_ARRAY=-13;
                int const SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY=-14;
                int const SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY=-15;
                int const SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY=-16;
                int const SerializationConstants::CONSTANT_TYPE_LONG_ARRAY=-17;
                int const SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY=-18;
                int const SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY=-19;
                int const SerializationConstants::CONSTANT_TYPE_STRING_ARRAY=-20;

                SerializationConstants::SerializationConstants()
                : size(21)
                , typeIdNameVector(size){
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_NULL)] = "null";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_PORTABLE)] = "portable";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_DATA)] = "data";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_BYTE)] = "byte";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_BOOLEAN)] = "boolean";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_CHAR)] = "char";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_SHORT)] = "short";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_INTEGER)] = "integer";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_LONG)] = "long";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_FLOAT)] = "float";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_DOUBLE)] = "double";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_STRING)] = "string";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_BYTE_ARRAY)] = "byteArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_BOOLEAN_ARRAY)] = "booleanArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_CHAR_ARRAY)] = "charArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_SHORT_ARRAY)] = "shortArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_INTEGER_ARRAY)] = "integerArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_LONG_ARRAY)] = "longArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_FLOAT_ARRAY)] = "floatArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_DOUBLE_ARRAY)] = "doubleArray";
                    typeIdNameVector[idToIndex(CONSTANT_TYPE_STRING_ARRAY)] = "stringArray";
                }

                std::string SerializationConstants::typeIdToName(int typeId) const{
                    int i = idToIndex(typeId);
                    if (i < 0 || i >= size)
                        return std::string("custom");
                    return typeIdNameVector[i];
                }

                void SerializationConstants::checkClassType(int expectedType, int currentType) const{
                    if (expectedType != currentType) {
                        char message[200];
                        util::snprintf(message, 200, "Received data of type %s(%d) but expected data type %s(%d)",
                        typeIdToName(currentType).c_str(), currentType,
                        typeIdToName(expectedType).c_str(), expectedType);

                        util::ILogger::getLogger().severe(message);
                        throw exception::IClassCastException("SerializationConstants::checkClassType",message);
                    }
                }

                int SerializationConstants::idToIndex(int id) const{
                    return id + size - 1;
                }

            }
        }
    }
}
