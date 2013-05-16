//
//  MorphingPortableReader.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "HazelcastException.h"
#include "MorphingPortableReader.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            MorphingPortableReader::MorphingPortableReader(SerializationContext *serializationContext, BufferedDataInput& input, boost::shared_ptr <ClassDefinition> cd)
            : input(input)
            , context(serializationContext)
            , cd(cd)
            , readingPortable(false)
            , raw(false)
            , offset(input.position()) {

            };

            MorphingPortableReader & MorphingPortableReader::operator [](std::string fieldName) {
                if (raw) {
                    throw hazelcast::client::HazelcastException("Cannot call [] operation after reading  directly from stream(without [])");
                }
                lastFieldName = fieldName;
                input.position(getPosition(fieldName));
                readingPortable = true;
                return *this;
            }

            //TODO need more thought on above and below functions
            void MorphingPortableReader::readingFromDataInput() {
                if (readingPortable) {
                    readingPortable = false;
                } else {
                    input.position(offset + cd->getFieldCount() * 4);
                    int pos = input.readInt();
                    input.position(pos);
                    raw = true;
                }
            };

            int MorphingPortableReader::readInt() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                FieldType id = fd.getType();
                if (id == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (id == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (id == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (id == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            long MorphingPortableReader::readLong() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                FieldType id = fd.getType();
                if (id == FieldTypes::TYPE_LONG) {
                    return input.readLong();
                } else if (id == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (id == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (id == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (id == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            bool MorphingPortableReader::readBoolean() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                if (fd.getType() != FieldTypes::TYPE_BOOLEAN)
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");

                return input.readBoolean();
            };

            byte MorphingPortableReader::readByte() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                if (fd.getType() != FieldTypes::TYPE_BYTE)
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");

                return input.readByte();
            };

            char MorphingPortableReader::readChar() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                if (fd.getType() != FieldTypes::TYPE_CHAR)
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");

                return input.readChar();
            };

            double MorphingPortableReader::readDouble() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                FieldType id = fd.getType();
                if (id == FieldTypes::TYPE_FLOAT) {
                    return input.readFloat();
                } else if (id == FieldTypes::TYPE_DOUBLE) {
                    return input.readDouble();
                } else if (id == FieldTypes::TYPE_LONG) {
                    return input.readLong();
                } else if (id == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (id == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (id == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (id == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            float MorphingPortableReader::readFloat() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                FieldType id = fd.getType();
                if (id == FieldTypes::TYPE_FLOAT) {
                    return input.readFloat();
                } else if (id == FieldTypes::TYPE_INT) {
                    return input.readInt();
                } else if (id == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                } else if (id == FieldTypes::TYPE_CHAR) {
                    return input.readChar();
                } else if (id == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            short MorphingPortableReader::readShort() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return 0;
                FieldDefinition fd = cd->get(lastFieldName);

                FieldType id = fd.getType();
                if (id == FieldTypes::TYPE_BYTE) {
                    return input.readByte();
                }
                if (id == FieldTypes::TYPE_SHORT) {
                    return input.readShort();
                } else {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
            };

            string MorphingPortableReader::readUTF() {

                if (!cd->isFieldDefinitionExists(lastFieldName))
                    return NULL;
                FieldDefinition fd = cd->get(lastFieldName);

                if (fd.getType() != FieldTypes::TYPE_UTF) {
                    throw hazelcast::client::HazelcastException("IncompatibleClassChangeError");
                }
                return input.readUTF();
            };

            int MorphingPortableReader::getPosition(std::string& fieldName) {
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
