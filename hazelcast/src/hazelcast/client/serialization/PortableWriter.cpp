//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "PortableWriter.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableWriter::PortableWriter(SerializationContext *serializationContext, ClassDefinition *cd, BufferedDataOutput *output)
            : context(serializationContext)
            , output(output)
            , index (0)
            , cd(cd)
            , raw(false)
            , offset(output->position()) {
                int const fieldIndexesLength = (cd->getFieldCount() + 1) * sizeof (int);
                this->output->position(offset + fieldIndexesLength);
            };

            void PortableWriter::writeInt(const char *fieldName, int value) {
                setPosition(fieldName);
                output->writeInt(value);
            };

            void PortableWriter::writeLong(const char *fieldName, long value) {
                setPosition(fieldName);
                output->writeLong(value);
            };

            void PortableWriter::writeBoolean(const char *fieldName, bool value) {
                setPosition(fieldName);
                output->writeBoolean(value);
            };

            void PortableWriter::writeByte(const char *fieldName, byte value) {
                setPosition(fieldName);
                output->writeByte(value);
            };

            void PortableWriter::writeChar(const char *fieldName, int value) {
                setPosition(fieldName);
                output->writeChar(value);
            };

            void PortableWriter::writeDouble(const char *fieldName, double value) {
                setPosition(fieldName);
                output->writeDouble(value);
            };

            void PortableWriter::writeFloat(const char *fieldName, float value) {
                setPosition(fieldName);
                output->writeFloat(value);
            };

            void PortableWriter::writeShort(const char *fieldName, short value) {
                setPosition(fieldName);
                output->writeShort(value);
            };

            void PortableWriter::writeUTF(const char *fieldName, const string& value) {
                setPosition(fieldName);
                output->writeUTF(value);
            };

            void PortableWriter::writeNullPortable(const char *fieldName, int factoryId, int classId) {
                setPosition(fieldName);
                output->writeBoolean(true);
            };

            void PortableWriter::writeByteArray(const char *fieldName, const vector<byte>  & bytes) {
                setPosition(fieldName);
                output->write(bytes);
            };

            void PortableWriter::writeCharArray(const char *fieldName, const std::vector<char >&  data) {
                setPosition(fieldName);
                int size = data.size();
                output->writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        output->writeChar(data[i]);
                    }
                }
            };

            void PortableWriter::writeShortArray(const char *fieldName, const std::vector<short >&  data) {
                setPosition(fieldName);
                int size = data.size();
                output->writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        output->writeShort(data[i]);
                    }
                }
            };

            void PortableWriter::writeIntArray(const char *fieldName, const std::vector<int>&  data) {
                setPosition(fieldName);
                int size = data.size();
                output->writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        output->writeInt(data[i]);
                    }
                }
            };

            void PortableWriter::writeLongArray(const char *fieldName, const std::vector<long >&  data) {
                setPosition(fieldName);
                int size = data.size();
                output->writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        output->writeLong(data[i]);
                    }
                }
            };

            void PortableWriter::writeFloatArray(const char *fieldName, const std::vector<float >&  data) {
                setPosition(fieldName);
                int size = data.size();
                output->writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        output->writeFloat(data[i]);
                    }
                }
            };

            void PortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double >&  data) {
                setPosition(fieldName);
                int size = data.size();
                output->writeInt(size);
                if (size > 0) {
                    for (int i = 0; i < size; i++) {
                        output->writeDouble(data[i]);
                    }
                }
            };

//            void PortableWriter::writeData(const char *fieldName, const Data&  data) { //TODO ???
//                setPosition(fieldName);
//                output->writeInt(data.getType());
//                if (data.cd != NULL) {
//                    output->writeInt(data.cd->getClassId());
//                    output->writeInt(data.cd->getFactoryId());
//                    output->writeInt(data.cd->getVersion());
//                    const std::vector<byte>& classDefBytes = data.cd->getBinary();
//                    output->writeInt(classDefBytes.size());
//                    output->write(classDefBytes);
//                } else {
//                    output->writeInt(data.NO_CLASS_ID);
//                }
//                int len = data.bufferSize();
//                output->writeInt(len);
//                if (len > 0) {
//                    output->write(*(data.buffer.get()));
//                }
//                output->writeInt(data.partitionHash);
//            };


            void PortableWriter::setPosition(const char *fieldName) {
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

            BufferedDataOutput* PortableWriter::getRawDataOutput() {
                if (!raw) {
                    int pos = output->position();
                    int index = cd->getFieldCount(); // last index
                    output->writeInt(offset + index * sizeof(int), pos);
                }
                raw = true;
                return output;
            };


        }
    }
}