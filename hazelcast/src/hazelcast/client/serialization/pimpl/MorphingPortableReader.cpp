//
//  MorphingPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/MorphingPortableReader.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"
#include "hazelcast/client/serialization/Portable.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                MorphingPortableReader::MorphingPortableReader(PortableContext&portableContext, DataInput &input, boost::shared_ptr<ClassDefinition> cd)
                : PortableReaderBase(portableContext, input, cd) {
                }

                int MorphingPortableReader::readInt(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return 0;
                    }

                    if (*currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName);
                    } else if (*currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName, true);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                long MorphingPortableReader::readLong(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return 0;
                    }

                    if (*currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName);
                    } else if (*currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName, true);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                bool MorphingPortableReader::readBoolean(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return false;
                    }
                    return PortableReaderBase::readBoolean(fieldName);
                }

                byte MorphingPortableReader::readByte(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return 0;
                    }
                    return PortableReaderBase::readByte(fieldName);
                }

                char MorphingPortableReader::readChar(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return 0;
                    }
                    return PortableReaderBase::readChar(fieldName);
                }

                double MorphingPortableReader::readDouble(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return 0.0;
                    }

                    if (*currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_DOUBLE) {
                        return PortableReaderBase::readDouble(fieldName);
                    } else if (*currentFieldType == FieldTypes::TYPE_LONG) {
                        return PortableReaderBase::readLong(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName, true);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                float MorphingPortableReader::readFloat(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return 0.0;
                    }

                    if (*currentFieldType == FieldTypes::TYPE_FLOAT) {
                        return PortableReaderBase::readFloat(fieldName);
                    } else if (*currentFieldType == FieldTypes::TYPE_INT) {
                        return PortableReaderBase::readInt(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_CHAR) {
                        return PortableReaderBase::readChar(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName, true);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                short MorphingPortableReader::readShort(char const *fieldName, bool skipTypeCheck) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return 0;
                    }

                    if (*currentFieldType == FieldTypes::TYPE_BYTE) {
                        return PortableReaderBase::readByte(fieldName, true);
                    } else if (*currentFieldType == FieldTypes::TYPE_SHORT) {
                        return PortableReaderBase::readShort(fieldName);
                    } else {
                        throw exception::HazelcastSerializationException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                    }
                }

                std::string MorphingPortableReader::readUTF(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return "";
                    }
                    return PortableReaderBase::readUTF(fieldName);
                }

                hazelcast::util::ByteVector_ptr MorphingPortableReader::readByteArray(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return hazelcast::util::ByteVector_ptr(new hazelcast::util::ByteVector(1, 0));
                    }
                    return PortableReaderBase::readByteArray(fieldName);
                }

                hazelcast::util::CharVector_ptr MorphingPortableReader::readCharArray(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return hazelcast::util::CharVector_ptr(new hazelcast::util::CharVector(1, 0));
                    }
                    return PortableReaderBase::readCharArray(fieldName);
                }

                std::vector<int> MorphingPortableReader::readIntArray(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return std::vector<int>(1, 0);
                    }
                    return PortableReaderBase::readIntArray(fieldName);
                }

                std::vector<long> MorphingPortableReader::readLongArray(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return std::vector<long>(1, 0);
                    }
                    return PortableReaderBase::readLongArray(fieldName);
                }

                std::vector<double> MorphingPortableReader::readDoubleArray(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return std::vector<double>(1, 0);
                    }
                    return PortableReaderBase::readDoubleArray(fieldName);
                }

                std::vector<float> MorphingPortableReader::readFloatArray(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return std::vector<float>(1, 0);
                    }
                    return PortableReaderBase::readFloatArray(fieldName);
                }

                std::vector<short> MorphingPortableReader::readShortArray(char const *fieldName) {
                    const FieldType *currentFieldType = cd->getFieldTypeIfExists(fieldName);
                    if (0 == currentFieldType){
                        return std::vector<short>(1, 0);
                    }
                    return PortableReaderBase::readShortArray(fieldName);
                }


            }
        }
    }
}

