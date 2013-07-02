//
//  MorphingPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "MorphingPortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            MorphingPortableReader::MorphingPortableReader(SerializationContext *serializationContext, BufferedDataInput& input, ClassDefinition *cd)
            : input(input)
            , context(serializationContext)
            , cd(cd)
            , raw(false)
            , offset(input.position())
            , currentFieldType(0) {

            };

            int MorphingPortableReader::readInt(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            long MorphingPortableReader::readLong(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_LONG) {
                    return input.readLong();
                } else if (currentFieldType == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            bool MorphingPortableReader::readBoolean(char const *fieldName) {
                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType != FieldTypes::TYPE_BOOLEAN)
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");

                return input.readBoolean();
            };

            byte MorphingPortableReader::readByte(char const *fieldName) {
                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType != FieldTypes::TYPE_BYTE)
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");

                return input.readByte();
            };

            char MorphingPortableReader::readChar(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;


                if (currentFieldType != FieldTypes::TYPE_CHAR)
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");

                return input.readChar();
            };

            double MorphingPortableReader::readDouble(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                    return input.readFloat();
                } else if (currentFieldType == FieldTypes::TYPE_DOUBLE) {
                    return input.readDouble();
                } else if (currentFieldType == FieldTypes::TYPE_LONG) {
                    return input.readLong();
                } else if (currentFieldType == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            float MorphingPortableReader::readFloat(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                    return input.readFloat();
                } else if (currentFieldType == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            short MorphingPortableReader::readShort(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;
                if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                }
                if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            string MorphingPortableReader::readUTF(char const *fieldName) {

                if (setPosition(fieldName))
                    return "";

                if (currentFieldType != FieldTypes::TYPE_UTF) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readUTF();
            };

            std::vector <byte> MorphingPortableReader::readByteArray(char const *fieldName) {
                if (setPosition(fieldName))
                    return std::vector<byte>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_BYTE_ARRAY) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readByteArray();
            };

            std::vector<char> MorphingPortableReader::readCharArray(char const *fieldName) {
                if (setPosition(fieldName))
                    return std::vector<char>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_CHAR_ARRAY) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readCharArray();
            };

            std::vector<int> MorphingPortableReader::readIntArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<int>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_INT_ARRAY) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readIntArray();
            };

            std::vector<long> MorphingPortableReader::readLongArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<long>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_LONG_ARRAY) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readLongArray();
            };

            std::vector<double> MorphingPortableReader::readDoubleArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<double>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_DOUBLE_ARRAY) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readDoubleArray();
            };

            std::vector<float> MorphingPortableReader::readFloatArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<float>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_FLOAT_ARRAY) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readFloatArray();
            };

            std::vector<short> MorphingPortableReader::readShortArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<short>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_SHORT_ARRAY) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readShortArray();
            };

            int MorphingPortableReader::getPosition(const char *fieldName) {
                input.position(offset + cd->get(fieldName).getIndex() * sizeof (int));
                return input.readInt();
            };

            bool MorphingPortableReader::setPosition(char const *fieldName) {
                if (cd->isFieldDefinitionExists(fieldName)) {
                    const FieldDefinition& fd = cd->get(fieldName);
                    currentFactoryId = fd.getFactoryId();
                    currentClassId = fd.getClassId();
                    input.position(getPosition(fieldName));
                    currentFieldType = cd->getFieldType(fieldName);
                    return false;
                } else {
                    return true;
                }
            }

            BufferedDataInput *MorphingPortableReader::getRawDataInput() {
                if (!raw) {
                    input.position(offset + cd->getFieldCount() * 4);
                    int pos = input.readInt();
                    input.position(pos);

                }
                raw = true;
                return &input;
            }

        }
    }
}
