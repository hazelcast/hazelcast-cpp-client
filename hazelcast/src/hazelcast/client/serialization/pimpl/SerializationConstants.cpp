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
// Created by sancar koyunlu on 21/02/14.
//

#include "hazelcast/client/serialization/pimpl/SerializationConstants.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                std::string SerializationConstants::typeIdToName(int32_t typeId) {
                    switch (typeId) {
                        case CONSTANT_TYPE_NULL:
                            return "null";
                        case CONSTANT_TYPE_PORTABLE:
                            return "portable";
                        case CONSTANT_TYPE_DATA:
                            return "data";
                        case CONSTANT_TYPE_BYTE:
                            return "byte";
                        case CONSTANT_TYPE_BOOLEAN:
                            return "boolean";
                        case CONSTANT_TYPE_CHAR:
                            return "char";
                        case CONSTANT_TYPE_SHORT:
                            return "short";
                        case CONSTANT_TYPE_INTEGER:
                            return "integer";
                        case CONSTANT_TYPE_LONG:
                            return "long";
                        case CONSTANT_TYPE_FLOAT:
                            return "float";
                        case CONSTANT_TYPE_DOUBLE:
                            return "double";
                        case CONSTANT_TYPE_STRING:
                            return "string";
                        case CONSTANT_TYPE_BYTE_ARRAY:
                            return "byteArray";
                        case CONSTANT_TYPE_BOOLEAN_ARRAY:
                            return "booleanArray";
                        case CONSTANT_TYPE_CHAR_ARRAY:
                            return "charArray";
                        case CONSTANT_TYPE_SHORT_ARRAY:
                            return "shortArray";
                        case CONSTANT_TYPE_INTEGER_ARRAY:
                            return "integerArray";
                        case CONSTANT_TYPE_LONG_ARRAY:
                            return "longArray";
                        case CONSTANT_TYPE_FLOAT_ARRAY:
                            return "floatArray";
                        case CONSTANT_TYPE_DOUBLE_ARRAY:
                            return "doubleArray";
                        case CONSTANT_TYPE_STRING_ARRAY:
                            return "stringArray";
                        default:
                            return "custom";
                    }
                }

                void SerializationConstants::checkClassType(int32_t expectedType, int32_t currentType) {
                    if (expectedType != currentType) {
                        char message[200];
                        util::hz_snprintf(message, 200, "Received data of type %s(%d) but expected data type %s(%d)",
                                          typeIdToName(currentType).c_str(), currentType,
                                          typeIdToName(expectedType).c_str(), expectedType);

                        util::ILogger::getLogger().severe(message);
                        throw exception::ClassCastException("SerializationConstants::checkClassType", message);
                    }
                }
            }
        }
    }
}
