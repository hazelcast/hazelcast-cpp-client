//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableWriter.h"
#include "Util.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableWriter::PortableWriter(SerializationContext *serializationContext, boost::shared_ptr<ClassDefinition> cd, BufferedDataOutput *output)
            : context(serializationContext)
            , output(output)
            , index (0)
            , cd(cd)
            , raw(false)
            , writingPortable(false) {
                offset = output->position();
                int const fieldIndexesLength = (cd->getFieldCount() + 1) * sizeof (int);
                this->output->position(offset + fieldIndexesLength);
            };


            PortableWriter& PortableWriter::operator [](std::string fieldName) {
                if (raw) {
                    throw hazelcast::client::HazelcastException("Cannot call [] operation after writing directly to stream(without [])");
                }
                setPosition(fieldName);
                writingPortable = true;
                return *this;
            };

            //TODO need more thought on above and below functions
            void PortableWriter::writingToDataOutput() {

                if (writingPortable) {
                    writingPortable = false;
                } else {
                    int pos = output->position();
                    int index = cd->getFieldCount(); // last index
                    output->writeInt(offset + index * sizeof(int), pos);
                    raw = true;
                }
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

            void PortableWriter::writeUTF(string value) {
                output->writeUTF(value);
            };

            void PortableWriter::writeNullPortable(int factoryId, int classId) {
                output->writeBoolean(true);
            };

            void PortableWriter::writeByteArray(std::vector<byte>& values) {
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeByte(values[i]);
                    }
                }
            };

            void PortableWriter::writeCharArray(std::vector<char>& values) {
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeChar(values[i]);
                    }
                }
            };

            void PortableWriter::writeIntArray(std::vector<int>& values) {
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeInt(values[i]);
                    }
                }
            };

            void PortableWriter::writeLongArray(std::vector<long>& values) {
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeLong(values[i]);
                    }
                }
            };

            void PortableWriter::writeDoubleArray(std::vector<double>& values) {
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeDouble(values[i]);
                    }
                }
            };

            void PortableWriter::writeFloatArray(std::vector<float>& values) {
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeFloat(values[i]);
                    }
                }
            };

            void PortableWriter::writeShortArray(std::vector<short>& values) {
                int len = values.size();
                output->writeInt(len);
                if (len > 0) {
                    for (int i = 0; i < len; i++) {
                        output->writeShort(values[i]);
                    }
                }
            };

            void PortableWriter::setPosition(string fieldName) {
                if (raw) throw HazelcastException("Cannot write Portable fields after getRawDataOutput() is called!");
                if (!cd->isFieldDefinitionExists(fieldName)) {
                    std::string error;
                    error += "HazelcastSerializationException( Invalid field name: '";
                    error += fieldName;
                    error += "' for ClassDefinition {id: "; //TODO add factory ID
                    error += hazelcast::client::util::to_string(cd->getClassId());
                    error += ", version: ";
                    error += hazelcast::client::util::to_string(cd->getVersion());
                    error += "}";

                    throw hazelcast::client::HazelcastException(error);
                }
                FieldDefinition fd = cd->get(fieldName);

                if (writtenFields.count(fieldName) != 0)
                    hazelcast::client::HazelcastException("Field '" + fieldName + "' has already been written!");
                writtenFields.insert(fieldName);
                int pos = output->position();
                int index = fd.getIndex();
                output->writeInt(offset + index * sizeof (int), pos);

            };
        }
    }
}