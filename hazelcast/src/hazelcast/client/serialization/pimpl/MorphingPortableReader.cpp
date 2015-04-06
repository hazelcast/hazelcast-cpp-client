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
                MorphingPortableReader::MorphingPortableReader(PortableContext&portableContext, DataInput &input, boost::shared_ptr<ClassDefinition> cd)
                : PortableReaderBase(portableContext, input, cd) {
                }

                int MorphingPortableReader::readInt(char const *fieldName) {
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

                long MorphingPortableReader::readLong(char const *fieldName) {
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

                short MorphingPortableReader::readShort(char const *fieldName) {
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

                std::string MorphingPortableReader::readUTF(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return "";
                    }
                    return PortableReaderBase::readUTF(fieldName);
                }

                std::vector<byte> MorphingPortableReader::readByteArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::vector<byte>(1, 0);
                    }
                    return PortableReaderBase::readByteArray(fieldName);
                }

                std::vector<char> MorphingPortableReader::readCharArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::vector<char>(1, 0);
                    }
                    return PortableReaderBase::readCharArray(fieldName);
                }

                std::vector<int> MorphingPortableReader::readIntArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::vector<int>(1, 0);
                    }
                    return PortableReaderBase::readIntArray(fieldName);
                }

                std::vector<long> MorphingPortableReader::readLongArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::vector<long>(1, 0);
                    }
                    return PortableReaderBase::readLongArray(fieldName);
                }

                std::vector<double> MorphingPortableReader::readDoubleArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::vector<double>(1, 0);
                    }
                    return PortableReaderBase::readDoubleArray(fieldName);
                }

                std::vector<float> MorphingPortableReader::readFloatArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::vector<float>(1, 0);
                    }
                    return PortableReaderBase::readFloatArray(fieldName);
                }

                std::vector<short> MorphingPortableReader::readShortArray(char const *fieldName) {
                    if (!cd->hasField(fieldName)){
                        return std::vector<short>(1, 0);
                    }
                    return PortableReaderBase::readShortArray(fieldName);
                }


            }
        }
    }
}

