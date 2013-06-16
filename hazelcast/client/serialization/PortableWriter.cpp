//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableWriter::PortableWriter(SerializationContext *serializationContext, ClassDefinition *cd, BufferedDataOutput *output)
            : context(serializationContext)
            , output(output)
            , index (0)
            , cd(cd)
            , raw(false)
            , writingPortable(false)
            , offset(output->position()) {
                int const fieldIndexesLength = (cd->getFieldCount() + 1) * sizeof (int);
                this->output->position(offset + fieldIndexesLength);
            };


//            PortableWriter& PortableWriter::operator [](const std::string& fieldName) {
            PortableWriter& PortableWriter::operator [](const char *fieldName) {
                if (raw) {
                    throw HazelcastException("Cannot call [] operation after writing directly to stream(without [])");
                }
                setPosition(fieldName);
                writingPortable = true;
                return *this;
            };

            void PortableWriter::writingToDataOutput() {

                if (writingPortable) {
                    writingPortable = false;
                } else if (!raw) {
                    int pos = output->position();
                    int index = cd->getFieldCount(); // last index
                    output->writeInt(offset + index * sizeof(int), pos);
                    raw = true;
                }
            };

            void PortableWriter::write(const vector<byte>  & bytes) {
                output->write(bytes);
            };

            void PortableWriter::writeInt(int value) {
                output->writeInt(value);
            };

            void PortableWriter::writeLong(long value) {
                output->writeLong(value);
            };

            void PortableWriter::writeBoolean(bool value) {
                output->writeBoolean(value);
            };

            void PortableWriter::writeByte(byte value) {
                output->writeByte(value);
            };

            void PortableWriter::writeChar(int value) {
                output->writeChar(value);
            };

            void PortableWriter::writeDouble(double value) {
                output->writeDouble(value);
            };

            void PortableWriter::writeFloat(float value) {
                output->writeFloat(value);
            };

            void PortableWriter::writeShort(short value) {
                output->writeShort(value);
            };

            void PortableWriter::writeUTF(const string& value) {
                output->writeUTF(value);
            };

            void PortableWriter::writeNullPortable(int factoryId, int classId) {
                output->writeBoolean(true);
            };

            void PortableWriter::setPosition(const char *fieldName) {
//            void PortableWriter::setPosition(const string& fieldName) {
                if (raw) throw HazelcastException("Cannot write Portable fields after getRawDataOutput() is called!");
                if (!cd->isFieldDefinitionExists(fieldName)) {
                    std::string error;
                    error += "HazelcastSerializationException( Invalid field name: '";
                    error += fieldName;
                    error += "' for ClassDefinition {id: "; //TODO add factory ID
                    error += hazelcast::util::to_string(cd->getClassId());
                    error += ", version: ";
                    error += hazelcast::util::to_string(cd->getVersion());
                    error += "}";

                    throw HazelcastException(error);
                }

                if (writtenFields.count(fieldName) != 0)
                    throw HazelcastException("Field '" + std::string(fieldName) + "' has already been written!");
                writtenFields.insert(fieldName);
                output->writeInt(offset + cd->get(fieldName).getIndex() * sizeof (int), output->position());

            };
        }
    }
}