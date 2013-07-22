//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/SerializationContext.h"
#include "hazelcast/client/serialization/PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableReader::PortableReader(SerializationContext *serializationContext, BufferedDataInput& input, boost::shared_ptr<ClassDefinition> cd)
            : input(input)
            , context(serializationContext)
            , cd(cd)
            , raw(false)
            , offset(input.position()) {

            };

            int PortableReader::readInt(const char *fieldName) {
                setPosition(fieldName);
                return input.readInt();
            };

            long PortableReader::readLong(const char *fieldName) {
                setPosition(fieldName);
                return input.readLong();
            };

            bool PortableReader::readBoolean(const char *fieldName) {
                setPosition(fieldName);
                return input.readBoolean();
            };

            byte PortableReader::readByte(const char *fieldName) {
                setPosition(fieldName);
                return input.readByte();
            };

            char PortableReader::readChar(const char *fieldName) {
                setPosition(fieldName);
                return input.readChar();
            };

            double PortableReader::readDouble(const char *fieldName) {
                setPosition(fieldName);
                return input.readDouble();
            };

            float PortableReader::readFloat(const char *fieldName) {
                setPosition(fieldName);
                return input.readFloat();
            };

            short PortableReader::readShort(const char *fieldName) {
                setPosition(fieldName);
                return input.readShort();
            };

            string PortableReader::readUTF(const char *fieldName) {
                setPosition(fieldName);
                return input.readUTF();
            };

            std::vector <byte> PortableReader::readByteArray(const char *fieldName) {
                setPosition(fieldName);
                return input.readByteArray();
            };

            std::vector<char> PortableReader::readCharArray(const char *fieldName) {
                setPosition(fieldName);
                return input.readCharArray();
            };

            std::vector<int> PortableReader::readIntArray(const char *fieldName) {
                setPosition(fieldName);
                return input.readIntArray();
            };

            std::vector<long> PortableReader::readLongArray(const char *fieldName) {
                setPosition(fieldName);
                return input.readLongArray();
            };

            std::vector<double> PortableReader::readDoubleArray(const char *fieldName) {
                setPosition(fieldName);
                return input.readDoubleArray();
            };

            std::vector<float> PortableReader::readFloatArray(const char *fieldName) {
                setPosition(fieldName);
                return input.readFloatArray();
            };

            std::vector<short> PortableReader::readShortArray(const char *fieldName) {
                setPosition(fieldName);
                return input.readShortArray();
            };


            void PortableReader::setPosition(char const *fieldName) {
                input.position(getPosition(fieldName));
                const FieldDefinition& fd = cd->get(fieldName);
                currentFactoryId = fd.getFactoryId();
                currentClassId = fd.getClassId();
            };

            int PortableReader::getPosition(const char *fieldName) {
                if (raw) {
                    throw exception::IException("PortableReader::getPosition ", "Cannot read Portable fields after getRawDataInput() is called!");
                }
                if (!cd->isFieldDefinitionExists(fieldName))
                    throw exception::IException("PortableReader::getPosition ", " unknownField " + std::string(fieldName));
                input.position(offset + cd->get(fieldName).getIndex() * sizeof (int));
                return input.readInt();
            };

            BufferedDataInput *PortableReader::getRawDataInput() {
                if (!raw) {
                    input.position(offset + cd->getFieldCount() * 4);
                    int pos = input.readInt();
                    input.position(pos);
                }
                raw = true;
                input.setSerializationContext(context);
                return &input;
            };

        }
    }
}
