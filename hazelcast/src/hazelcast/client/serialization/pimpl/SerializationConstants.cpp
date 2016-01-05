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

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                SerializationConstants *SerializationConstants::instance = NULL;

                SerializationConstants::SerializationConstants()
                : CONSTANT_TYPE_NULL(0)
                , CONSTANT_TYPE_PORTABLE(-1)
                , CONSTANT_TYPE_DATA(-2)
                , CONSTANT_TYPE_BYTE(-3)
                , CONSTANT_TYPE_BOOLEAN(-4)
                , CONSTANT_TYPE_CHAR(-5)
                , CONSTANT_TYPE_SHORT(-6)
                , CONSTANT_TYPE_INTEGER(-7)
                , CONSTANT_TYPE_LONG(-8)
                , CONSTANT_TYPE_FLOAT(-9)
                , CONSTANT_TYPE_DOUBLE(-10)
                , CONSTANT_TYPE_STRING(-11)
                , CONSTANT_TYPE_BYTE_ARRAY(-12)
                , CONSTANT_TYPE_BOOLEAN_ARRAY(-13)
                , CONSTANT_TYPE_CHAR_ARRAY(-14)
                , CONSTANT_TYPE_SHORT_ARRAY(-15)
                , CONSTANT_TYPE_INTEGER_ARRAY(-16)
                , CONSTANT_TYPE_LONG_ARRAY(-17)
                , CONSTANT_TYPE_FLOAT_ARRAY(-18)
                , CONSTANT_TYPE_DOUBLE_ARRAY(-19)
                , CONSTANT_TYPE_STRING_ARRAY(-20)
                , size(21)
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

                std::string SerializationConstants::typeIdToName(int typeId) {
                    int i = idToIndex(typeId);
                    if (i < 0 || i >= size)
                        return std::string("custom");
                    return typeIdNameVector[i];
                }

                int SerializationConstants::idToIndex(int id) {
                    return id + size - 1;
                }

                SerializationConstants *SerializationConstants::getInstance() {
                    if (NULL == instance) {
                        instance = new SerializationConstants();
                    }
                    return instance;
                }
            }
        }
    }
}
