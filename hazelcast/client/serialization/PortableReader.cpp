//
//  PortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "SerializationContext.h"
#import "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableReader::PortableReader(SerializationContext *serializationContext, BufferedDataInput& input, boost::shared_ptr <ClassDefinition> cd)
            : input(&input)
            , context(serializationContext)
            , cd(cd)
            , readingPortable(false)
            , raw(false) {

            };

            PortableReader & PortableReader::operator [](string & fieldName) {
                if (raw) {
                    throw hazelcast::client::HazelcastException("Cannot call [] operation after reading  directly from stream(without [])");
                }
                lastFieldName = fieldName;
                input->position(getPosition(fieldName));
                return *this;
            }

            //TODO need more thought on above and below functions
            void PortableReader::readingFromDataInput() {
                if (readingPortable) {
                    readingPortable = false;
                } else {
                    int pos = input->readInt(offset + cd->getFieldCount() * 4);
                    input->position(pos);
                    raw = true;
                }
            };

            int PortableReader::readInt() {
                return input->readInt();
            };

            long PortableReader::readLong() {
                return input->readLong();
            };

            bool PortableReader::readBoolean() {
                return input->readBoolean();
            };

            byte PortableReader::readByte() {
                return input->readByte();
            };

            char PortableReader::readChar() {
                return input->readChar();
            };

            double PortableReader::readDouble() {
                return input->readDouble();
            };

            float PortableReader::readFloat() {
                return input->readFloat();
            };

            short PortableReader::readShort() {
                return input->readShort();
            };

            string PortableReader::readUTF() {
                return input->readUTF();
            };

            std::vector <byte> PortableReader::readByteArray() {
                input->position();
                int len = input->readInt();
                std::vector <byte> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readByte();
                }
                return values;
            };

            std::vector<char> PortableReader::readCharArray() {
                input->position();
                int len = input->readInt();
                std::vector<char> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readChar();
                }
                return values;
            };

            std::vector<int> PortableReader::readIntArray() {
                input->position();
                int len = input->readInt();
                std::vector<int> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readInt();
                }
                return values;
            };

            std::vector<long> PortableReader::readLongArray() {
                input->position();
                int len = input->readInt();
                std::vector<long> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readLong();
                }
                return values;
            };

            std::vector<double> PortableReader::readDoubleArray() {
                input->position();
                int len = input->readInt();
                std::vector<double> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readDouble();
                }
                return values;
            };

            std::vector<float> PortableReader::readFloatArray() {
                input->position();
                int len = input->readInt();
                std::vector<float> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readFloat();
                }
                return values;
            };

            std::vector<short> PortableReader::readShortArray() {
                input->position();
                int len = input->readInt();
                std::vector<short> values(len);
                for (int i = 0; i < len; i++) {
                    values[i] = input->readShort();
                }
                return values;
            };

            int PortableReader::getPosition(std::string& fieldName) {
                if (raw) {
                    throw hazelcast::client::HazelcastException("Cannot read Portable fields after getRawDataInput() is called!");
                }

                if (!cd->isFieldDefinitionExists(fieldName))
                    throw hazelcast::client::HazelcastException("PortableReader::getPosition : unknownField " + fieldName);
                FieldDefinition fd = cd->get(fieldName);
                return getPosition(&fd);
            };

            int PortableReader::getPosition(FieldDefinition *fd) {
                return input->readInt(offset + fd->getIndex() * sizeof (int));
            };

        }
    }
}
