//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/SerializationContext.h"
#include "DefaultPortableReader.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            DefaultPortableReader::DefaultPortableReader(SerializerHolder& serializerHolder, SerializationContext& serializationContext, DataInput& input, util::AtomicPointer<ClassDefinition> cd)
            :serializerHolder(serializerHolder)
            , context(serializationContext)
            , dataInput(input)
            , objectDataInput(input, serializerHolder, serializationContext)
            , finalPosition(input.readInt()) //TODO what happens in case of exception
            , offset(input.position())
            , cd(cd)
            , raw(false) {

            };

            int DefaultPortableReader::readInt(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readInt();
            };

            long DefaultPortableReader::readLong(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readLong();
            };

            bool DefaultPortableReader::readBoolean(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readBoolean();
            };

            byte DefaultPortableReader::readByte(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readByte();
            };

            char DefaultPortableReader::readChar(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readChar();
            };

            double DefaultPortableReader::readDouble(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readDouble();
            };

            float DefaultPortableReader::readFloat(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readFloat();
            };

            short DefaultPortableReader::readShort(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readShort();
            };

            string DefaultPortableReader::readUTF(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readUTF();
            };

            std::vector <byte> DefaultPortableReader::readByteArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readByteArray();
            };

            std::vector<char> DefaultPortableReader::readCharArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readCharArray();
            };

            std::vector<int> DefaultPortableReader::readIntArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readIntArray();
            };

            std::vector<long> DefaultPortableReader::readLongArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readLongArray();
            };

            std::vector<double> DefaultPortableReader::readDoubleArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readDoubleArray();
            };

            std::vector<float> DefaultPortableReader::readFloatArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readFloatArray();
            };

            std::vector<short> DefaultPortableReader::readShortArray(const char *fieldName) {
                setPosition(fieldName);
                return dataInput.readShortArray();
            };


            void DefaultPortableReader::setPosition(char const *fieldName) {
                dataInput.position(getPosition(fieldName));
                const FieldDefinition& fd = cd->get(fieldName);
                currentFactoryId = fd.getFactoryId();
                currentClassId = fd.getClassId();
            };

            int DefaultPortableReader::getPosition(const char *fieldName) {
                if (raw) {
                    throw exception::IException("PortableReader::getPosition ", "Cannot read Portable fields after getRawDataInput() is called!");
                }
                if (!cd->isFieldDefinitionExists(fieldName))
                    throw exception::IException("PortableReader::getPosition ", " unknownField " + std::string(fieldName));
                dataInput.position(offset + cd->get(fieldName).getIndex() * sizeof (int));
                return dataInput.readInt();
            };

            ObjectDataInput& DefaultPortableReader::getRawDataInput() {
                if (!raw) {
                    dataInput.position(offset + cd->getFieldCount() * 4);
                    int pos = dataInput.readInt();
                    dataInput.position(pos);
                }
                raw = true;
                return objectDataInput; //TODO why return pointer not reference
            };

            void DefaultPortableReader::read(DataInput& dataInput, Portable& object, int factoryId, int classId) {
                serializerHolder.getPortableSerializer().read(dataInput, object, factoryId, classId, cd->getVersion());
            };

            void DefaultPortableReader::end() {
                dataInput.position(finalPosition);
            };

        }
    }
}
