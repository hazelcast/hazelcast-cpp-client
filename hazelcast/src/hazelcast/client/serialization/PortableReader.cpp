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

            PortableReader::PortableReader(SerializationContext& serializationContext, DataInput& input, util::AtomicPointer<ClassDefinition> cd)
            : dataInput(input)
            , context(serializationContext)
            , objectDataInput(input, serializationContext),
            , finalPosition(input.readInt()) //TODO what happens in case of exception
            , offset(input.position())
            , cd(cd)
            , raw(false) {

            };

            int PortableReader::readInt(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readInt();
            };

            long PortableReader::readLong(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readLong();
            };

            bool PortableReader::readBoolean(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readBoolean();
            };

            byte PortableReader::readByte(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readByte();
            };

            char PortableReader::readChar(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readChar();
            };

            double PortableReader::readDouble(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readDouble();
            };

            float PortableReader::readFloat(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readFloat();
            };

            short PortableReader::readShort(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readShort();
            };

            string PortableReader::readUTF(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readUTF();
            };

            std::vector <byte> PortableReader::readByteArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readByteArray();
            };

            std::vector<char> PortableReader::readCharArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readCharArray();
            };

            std::vector<int> PortableReader::readIntArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readIntArray();
            };

            std::vector<long> PortableReader::readLongArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readLongArray();
            };

            std::vector<double> PortableReader::readDoubleArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readDoubleArray();
            };

            std::vector<float> PortableReader::readFloatArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readFloatArray();
            };

            std::vector<short> PortableReader::readShortArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readShortArray();
            };


            void PortableReader::setPosition(char const *fieldName) {
                dataInput.position(getPosition(fieldName));
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
                dataInput.position(offset + cd->get(fieldName).getIndex() * sizeof (int));
                return dataInput.readInt();
            };

            ObjectDataInput *PortableReader::getRawDataInput() {
                if (!raw) {
                    dataInput.position(offset + cd->getFieldCount() * 4);
                    int pos = dataInput.readInt();
                    dataInput.position(pos);
                }
                raw = true;
                dataInput.setSerializationContext(&context);
                return &objectDataInput; //TODO why return pointer not reference
            };

            void PortableReader::end() {
                dataInput.position(finalPosition);
            };

        }
    }
}
