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
            : input(input)
            , context(serializationContext)
            , cd(cd)
            , readingPortable(false)
            , raw(false)
            , offset(input.position()) {

            };

            PortableReader & PortableReader::operator [](std::string fieldName) {
                if (raw) {
                    throw hazelcast::client::HazelcastException("Cannot call [] operation after reading  directly from stream(without [])");
                }
                lastFieldName = fieldName;
                input.position(getPosition(fieldName));
                readingPortable = true;
                return *this;
            }

            //TODO need more thought on above and below functions
            void PortableReader::readingFromDataInput() {
                if (readingPortable) {
                    readingPortable = false;
                } else if (!raw) {
                    input.position(offset + cd->getFieldCount() * 4);
                    int pos = input.readInt();
                    input.position(pos);
                    raw = true;
                }
            };

            int PortableReader::readInt() {
                return input.readInt();
            };

            long PortableReader::readLong() {
                return input.readLong();
            };

            bool PortableReader::readBoolean() {
                return input.readBoolean();
            };

            byte PortableReader::readByte() {
                return input.readByte();
            };

            char PortableReader::readChar() {
                return input.readChar();
            };

            double PortableReader::readDouble() {
                return input.readDouble();
            };

            float PortableReader::readFloat() {
                return input.readFloat();
            };

            short PortableReader::readShort() {
                return input.readShort();
            };

            string PortableReader::readUTF() {
                return input.readUTF();
            };

            std::vector <byte> PortableReader::readByteArray() {
                return input.readByteArray();
            };

            std::vector<char> PortableReader::readCharArray() {
                return input.readCharArray();
            };

            std::vector<int> PortableReader::readIntArray() {
                return input.readIntArray();
            };

            std::vector<long> PortableReader::readLongArray() {
                return input.readLongArray();
            };

            std::vector<double> PortableReader::readDoubleArray() {
                return input.readDoubleArray();
            };

            std::vector<float> PortableReader::readFloatArray() {
                return input.readFloatArray();
            };

            std::vector<short> PortableReader::readShortArray() {
                return input.readShortArray();
            };

            int PortableReader::getPosition(std::string& fieldName) {
                if (raw) {
                    throw hazelcast::client::HazelcastException("Cannot read Portable fields after getRawDataInput() is called!");
                }

                if (!cd->isFieldDefinitionExists(fieldName))
                    throw hazelcast::client::HazelcastException("PortableReader::getPosition : unknownField " + fieldName);
                FieldDefinition fd = cd->get(fieldName);
                input.position(offset + fd.getIndex() * sizeof (int));
                return input.readInt();
            };

        }
    }
}
