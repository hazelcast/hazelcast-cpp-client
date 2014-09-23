//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/PortableContext.h"
#include "hazelcast/client/serialization/pimpl/DefaultPortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                DefaultPortableReader::DefaultPortableReader(PortableContext& portableContext, DataInput& input, boost::shared_ptr<ClassDefinition> cd)
                : serializerHolder(portableContext.getSerializerHolder())
                , dataInput(input)
                , objectDataInput(input, portableContext)
                , finalPosition(input.readInt())
                , offset(input.position())
                , raw(false)
                , cd(cd) {

                }

                int DefaultPortableReader::readInt(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT);
                    return dataInput.readInt();
                }

                long DefaultPortableReader::readLong(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG);
                    return (long)dataInput.readLong();
                }

                bool DefaultPortableReader::readBoolean(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BOOLEAN);
                    return dataInput.readBoolean();
                }

                byte DefaultPortableReader::readByte(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE);
                    return dataInput.readByte();
                }

                char DefaultPortableReader::readChar(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR);
                    return dataInput.readChar();
                }

                double DefaultPortableReader::readDouble(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE);
                    return dataInput.readDouble();
                }

                float DefaultPortableReader::readFloat(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT);
                    return dataInput.readFloat();
                }

                short DefaultPortableReader::readShort(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT);
                    return dataInput.readShort();
                }

                std::string DefaultPortableReader::readUTF(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_UTF);
                    return dataInput.readUTF();
                }

                std::vector<byte> DefaultPortableReader::readByteArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    return dataInput.readByteArray();
                }

                std::vector<char> DefaultPortableReader::readCharArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    return dataInput.readCharArray();
                }

                std::vector<int> DefaultPortableReader::readIntArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_INT_ARRAY);
                    return dataInput.readIntArray();
                }

                std::vector<long> DefaultPortableReader::readLongArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    return dataInput.readLongArray();
                }

                std::vector<double> DefaultPortableReader::readDoubleArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    return dataInput.readDoubleArray();
                }

                std::vector<float> DefaultPortableReader::readFloatArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    return dataInput.readFloatArray();
                }

                std::vector<short> DefaultPortableReader::readShortArray(const char *fieldName) {
                    setPosition(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    return dataInput.readShortArray();
                }


                void DefaultPortableReader::setPosition(char const *fieldName, FieldType const& fieldType) {
                    dataInput.position(readPosition(fieldName, fieldType));
                }

                int DefaultPortableReader::readPosition(const char *fieldName, FieldType const& fieldType) {
                    if (raw) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ", "Cannot read Portable fields after getRawDataInput() is called!");
                    }
                    if (!cd->hasField(fieldName)) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ", "Don't have a field named " + std::string(fieldName));
                    }

                    if (cd->getFieldType(fieldName) != fieldType) {
                        throw exception::HazelcastSerializationException("PortableReader::getPosition ", "Field type did not matched for " + std::string(fieldName));
                    }
                    dataInput.position(offset + cd->getField(fieldName).getIndex() * sizeof(int));
                    return dataInput.readInt();
                }

                ObjectDataInput& DefaultPortableReader::getRawDataInput() {
                    if (!raw) {
                        dataInput.position(offset + cd->getFieldCount() * 4);
                        int pos = dataInput.readInt();
                        dataInput.position(pos);
                    }
                    raw = true;
                    return objectDataInput;
                }

                void DefaultPortableReader::read(DataInput& dataInput, Portable& object, int factoryId, int classId) {
                    serializerHolder.getPortableSerializer().read(dataInput, object, factoryId, classId, cd->getVersion());
                }

                void DefaultPortableReader::end() {
                    dataInput.position(finalPosition);
                }

            }
        }
    }
}

