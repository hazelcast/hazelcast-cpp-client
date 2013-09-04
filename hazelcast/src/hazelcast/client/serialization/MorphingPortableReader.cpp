//
//  MorphingPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/MorphingPortableReader.h"
#include "hazelcast/client/serialization/DefaultPortableReader.h"
#include "hazelcast/client/serialization/SerializationContext.h"
#include "hazelcast/client/Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            MorphingPortableReader::MorphingPortableReader(SerializationContext & serializationContext, DataInput& input, ClassDefinition *cd)
            : context(serializationContext)
            , serializerHolder(serializationContext.getSerializerHolder())
            , dataInput(input)
            , objectDataInput(input, serializationContext)
            , finalPosition(input.readInt())
            , offset(input.position())
            , cd(cd)
            , raw(false)
            , currentFieldType(0) {

            };

            int MorphingPortableReader::readInt(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_INT) {
                    return dataInput.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return dataInput.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return dataInput.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return dataInput.readShort();
                } else {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
            };

            long MorphingPortableReader::readLong(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_LONG) {
                    return dataInput.readLong();
                } else if (currentFieldType == FieldTypes::TYPE_INT) {
                    return dataInput.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return dataInput.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return dataInput.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return dataInput.readShort();
                } else {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
            };

            bool MorphingPortableReader::readBoolean(char const *fieldName) {
                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType != FieldTypes::TYPE_BOOLEAN)
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");

                return dataInput.readBoolean();
            };

            byte MorphingPortableReader::readByte(char const *fieldName) {
                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType != FieldTypes::TYPE_BYTE)
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");

                return dataInput.readByte();
            };

            char MorphingPortableReader::readChar(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;


                if (currentFieldType != FieldTypes::TYPE_CHAR)
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");

                return dataInput.readChar();
            };

            double MorphingPortableReader::readDouble(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                    return dataInput.readFloat();
                } else if (currentFieldType == FieldTypes::TYPE_DOUBLE) {
                    return dataInput.readDouble();
                } else if (currentFieldType == FieldTypes::TYPE_LONG) {
                    return dataInput.readLong();
                } else if (currentFieldType == FieldTypes::TYPE_INT) {
                    return dataInput.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return dataInput.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return dataInput.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return dataInput.readShort();
                } else {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
            };

            float MorphingPortableReader::readFloat(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;

                if (currentFieldType == FieldTypes::TYPE_FLOAT) {
                    return dataInput.readFloat();
                } else if (currentFieldType == FieldTypes::TYPE_INT) {
                    return dataInput.readInt();
                } else if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return dataInput.readByte();
                } else if (currentFieldType == FieldTypes::TYPE_CHAR) {
                    return dataInput.readChar();
                } else if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return dataInput.readShort();
                } else {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
            };

            short MorphingPortableReader::readShort(char const *fieldName) {

                if (setPosition(fieldName))
                    return 0;
                if (currentFieldType == FieldTypes::TYPE_BYTE) {
                    return dataInput.readByte();
                }
                if (currentFieldType == FieldTypes::TYPE_SHORT) {
                    return dataInput.readShort();
                } else {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
            };

            string MorphingPortableReader::readUTF(char const *fieldName) {

                if (setPosition(fieldName))
                    return "";

                if (currentFieldType != FieldTypes::TYPE_UTF) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readUTF();
            };

            std::vector <byte> MorphingPortableReader::readByteArray(char const *fieldName) {
                if (setPosition(fieldName))
                    return std::vector<byte>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_BYTE_ARRAY) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readByteArray();
            };

            std::vector<char> MorphingPortableReader::readCharArray(char const *fieldName) {
                if (setPosition(fieldName))
                    return std::vector<char>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_CHAR_ARRAY) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readCharArray();
            };

            std::vector<int> MorphingPortableReader::readIntArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<int>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_INT_ARRAY) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readIntArray();
            };

            std::vector<long> MorphingPortableReader::readLongArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<long>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_LONG_ARRAY) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readLongArray();
            };

            std::vector<double> MorphingPortableReader::readDoubleArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<double>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_DOUBLE_ARRAY) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readDoubleArray();
            };

            std::vector<float> MorphingPortableReader::readFloatArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<float>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_FLOAT_ARRAY) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readFloatArray();
            };

            std::vector<short> MorphingPortableReader::readShortArray(char const *fieldName) {
                if (setPosition(fieldName))
                    std::vector<short>(1, 0);

                if (currentFieldType != FieldTypes::TYPE_SHORT_ARRAY) {
                    throw exception::IOException("MorphingPortableReader::*", "IncompatibleClassChangeError");
                }
                return dataInput.readShortArray();
            };

            int MorphingPortableReader::getPosition(const char *fieldName) {
                dataInput.position(offset + cd->get(fieldName).getIndex() * sizeof (int));
                return dataInput.readInt();
            };

            bool MorphingPortableReader::setPosition(char const *fieldName) {
                if (cd->isFieldDefinitionExists(fieldName)) {
                    const FieldDefinition& fd = cd->get(fieldName);
                    currentFactoryId = fd.getFactoryId();
                    currentClassId = fd.getClassId();
                    dataInput.position(getPosition(fieldName));
                    currentFieldType = cd->getFieldType(fieldName);
                    return false;
                } else {
                    return true;
                }
            }

            ObjectDataInput& MorphingPortableReader::getRawDataInput() {
                if (!raw) {
                    dataInput.position(offset + cd->getFieldCount() * 4);
                    int pos = dataInput.readInt();
                    dataInput.position(pos);

                }
                raw = true;
                return objectDataInput;
            };

            void MorphingPortableReader::end() {
                dataInput.position(finalPosition);
            };

            void MorphingPortableReader::read(DataInput& dataInput, Portable& object, int factoryId, int classId, int dataVersion) {
                serializerHolder.getPortableSerializer().read(dataInput, object, factoryId, classId, dataVersion);
            };

        }
    }
}
