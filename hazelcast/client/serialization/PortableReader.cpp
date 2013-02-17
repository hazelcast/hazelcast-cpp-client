//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Portable.h"
#include "PortableReader.h"
#include "DataInput.h"
#include "PortableSerializer.h"
#include "FieldDefinition.h"

#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableReader::PortableReader(PortableSerializer* serializer, DataInput& input, ClassDefinition* cd)
            : serializer(serializer)
            , input(&input)
            , cd(cd)
            , offset(input.position()) {
            };

            int PortableReader::readInt(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readInt(pos);
            };

            long PortableReader::readLong(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readLong(pos);
            };

            bool PortableReader::readBoolean(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readBoolean(pos);
            };

            byte PortableReader::readByte(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readByte(pos);
            };

            char PortableReader::readChar(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readChar(pos);
            };

            double PortableReader::readDouble(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readDouble(pos);
            };

            float PortableReader::readFloat(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readFloat(pos);
            };

            short PortableReader::readShort(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readShort(pos);
            };

            string PortableReader::readUTF(string fieldName) {
                int pos = getPosition(fieldName);
                return input->readUTF(pos);
            };

            std::vector<byte> PortableReader::readByteArray(string fieldName) {
                int pos = getPosition(fieldName);
                input->position(pos);
                int len = input->readInt();
                std::vector<byte> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readByte();
                }
                return values;
            };

            std::vector<char> PortableReader::readCharArray(string fieldName) {
                int pos = getPosition(fieldName);
                input->position(pos);
                int len = input->readInt();
                std::vector<char> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readChar();
                }
                return values;
            };

            std::vector<int> PortableReader::readIntArray(string fieldName) {
                int pos = getPosition(fieldName);
                input->position(pos);
                int len = input->readInt();
                std::vector<int> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readInt();
                }
                return values;
            };

            std::vector<long> PortableReader::readLongArray(string fieldName) {
                int pos = getPosition(fieldName);
                input->position(pos);
                int len = input->readInt();
                std::vector<long> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readLong();
                }
                return values;
            };

            std::vector<double> PortableReader::readDoubleArray(string fieldName) {
                int pos = getPosition(fieldName);
                input->position(pos);
                int len = input->readInt();
                std::vector<double> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readDouble();
                }
                return values;
            };

            std::vector<float> PortableReader::readFloatArray(string fieldName) {
                int pos = getPosition(fieldName);
                input->position(pos);
                int len = input->readInt();
                std::vector<float> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readFloat();
                }
                return values;
            };

            std::vector<short> PortableReader::readShortArray(string fieldName) {
                int pos = getPosition(fieldName);
                input->position(pos);
                int len = input->readInt();
                std::vector<short> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readShort();
                }
                return values;
            };

            int PortableReader::getPosition(string fieldName) {
                if (!cd->isFieldDefinitionExists(fieldName))
                    throw "throwUnknownFieldException" + fieldName;
                FieldDefinition fd = cd->get(fieldName);
                return getPosition(&fd);
            };

            int PortableReader::getPosition(FieldDefinition* fd) {
                return input->readInt(offset + fd->getIndex() * sizeof (int));
            };

        }
    }
}
