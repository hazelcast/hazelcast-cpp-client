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
//  MorphingPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/MorphingPortableReader.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                MorphingPortableReader::MorphingPortableReader(PortableContext&portableContext, ObjectDataInput &input, boost::shared_ptr<ClassDefinition> cd)
                : PortableReaderBase(portableContext, input, cd) {
                }

                int32_t MorphingPortableReader::readInt(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return 0;
                    }
                    const FieldType& currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                int64_t MorphingPortableReader::readLong(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return 0;
                    }
                    const FieldType& currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                bool MorphingPortableReader::readBoolean(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return false;
                    }
                    return PortableReaderBase::readBoolean(fieldName);
                }

                byte MorphingPortableReader::readByte(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return 0;
                    }
                    return PortableReaderBase::readByte(fieldName);
                }

                char MorphingPortableReader::readChar(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return 0;
                    }

                    return PortableReaderBase::readChar(fieldName);
                }

                double MorphingPortableReader::readDouble(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return 0.0;
                    }
                    const FieldType& currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_DOUBLE) {
                        return PortableReaderBase::readDouble(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                float MorphingPortableReader::readFloat(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return 0.0;
                    }
                    const FieldType& currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_INT) {
                        return (float)PortableReaderBase::readInt(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
						return (float)PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
						return (float)PortableReaderBase::readChar(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
						return (float)PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                int16_t MorphingPortableReader::readShort(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return 0;
                    }
                    const FieldType& currentFieldType = cd->getFieldType(fieldName);

                    if (currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName);
                    } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                std::auto_ptr<std::string> MorphingPortableReader::readUTF(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::string>(new std::string(""));
                    }
                    return PortableReaderBase::readUTF(fieldName);
                }

                std::auto_ptr<std::vector<byte> > MorphingPortableReader::readByteArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::vector<byte> >(new std::vector<byte>(1, 0));
                    }
                    return PortableReaderBase::readByteArray(fieldName);
                }

                std::auto_ptr<std::vector<char> > MorphingPortableReader::readCharArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::vector<char> >(new std::vector<char>(1, 0));
                    }
                    return PortableReaderBase::readCharArray(fieldName);
                }

                std::auto_ptr<std::vector<int32_t> > MorphingPortableReader::readIntArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::vector<int32_t> >(new std::vector<int32_t>(1, 0));
                    }
                    return PortableReaderBase::readIntArray(fieldName);
                }

                std::auto_ptr<std::vector<int64_t> > MorphingPortableReader::readLongArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::vector<int64_t> >(new std::vector<int64_t>(1, 0));
                    }
                    return PortableReaderBase::readLongArray(fieldName);
                }

                std::auto_ptr<std::vector<double> > MorphingPortableReader::readDoubleArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::vector<double> >(new std::vector<double>(1, 0));
                    }
                    return PortableReaderBase::readDoubleArray(fieldName);
                }

                std::auto_ptr<std::vector<float> > MorphingPortableReader::readFloatArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::vector<float> >(new std::vector<float>(1, 0));
                    }
                    return PortableReaderBase::readFloatArray(fieldName);
                }

                std::auto_ptr<std::vector<int16_t> > MorphingPortableReader::readShortArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::auto_ptr<std::vector<int16_t> >(new std::vector<int16_t>(1, 0));
                    }
                    return PortableReaderBase::readShortArray(fieldName);
                }


            }
        }
    }
}

