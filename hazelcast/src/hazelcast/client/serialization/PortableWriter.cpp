//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "hazelcast/client/serialization/PortableWriter.h"
#include "IOException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            PortableWriter::PortableWriter(SerializationContext& serializationContext, util::AtomicPointer<ClassDefinition> cd, DataOutput& dataOutput)
            : context(serializationContext)
            , objectDataOutput(objectDataOutput)
            , dataOutput(dataOutput)
            , index (0)
            , cd(cd)
            , raw(false)
            , begin(dataOutput.position())
            , offset(dataOutput.position() + sizeof(int)) {
                int const fieldIndexesLength = (cd->getFieldCount() + 1) * sizeof (int);
                this->dataOutput.position(offset + fieldIndexesLength);
            };

            void PortableWriter::writeInt(const char *fieldName, int value) {
                setPosition(fieldName);
                dataOutput.writeInt(value);
            };

            void PortableWriter::writeLong(const char *fieldName, long value) {
                setPosition(fieldName);
                dataOutput.writeLong(value);
            };

            void PortableWriter::writeBoolean(const char *fieldName, bool value) {
                setPosition(fieldName);
                dataOutput.writeBoolean(value);
            };

            void PortableWriter::writeByte(const char *fieldName, byte value) {
                setPosition(fieldName);
                dataOutput.writeByte(value);
            };

            void PortableWriter::writeChar(const char *fieldName, int value) {
                setPosition(fieldName);
                dataOutput.writeChar(value);
            };

            void PortableWriter::writeDouble(const char *fieldName, double value) {
                setPosition(fieldName);
                dataOutput.writeDouble(value);
            };

            void PortableWriter::writeFloat(const char *fieldName, float value) {
                setPosition(fieldName);
                dataOutput.writeFloat(value);
            };

            void PortableWriter::writeShort(const char *fieldName, short value) {
                setPosition(fieldName);
                dataOutput.writeShort(value);
            };

            void PortableWriter::writeUTF(const char *fieldName, const string& value) {
                setPosition(fieldName);
                dataOutput.writeUTF(value);
            };

            void PortableWriter::writeNullPortable(const char *fieldName, int factoryId, int classId) {
                setPosition(fieldName);
                dataOutput.writeBoolean(true);
            };

            void PortableWriter::writeByteArray(const char *fieldName, const vector<byte>  & bytes) {
                setPosition(fieldName);
                dataOutput.writeByteArray(bytes);
            };

            void PortableWriter::writeCharArray(const char *fieldName, const std::vector<char >&  data) {
                setPosition(fieldName);
                dataOutput.writeCharArray(data);
            };

            void PortableWriter::writeShortArray(const char *fieldName, const std::vector<short >&  data) {
                setPosition(fieldName);
                dataOutput.writeShortArray(data);
            };

            void PortableWriter::writeIntArray(const char *fieldName, const std::vector<int>&  data) {
                setPosition(fieldName);
                dataOutput.writeIntArray(data);
            };

            void PortableWriter::writeLongArray(const char *fieldName, const std::vector<long >&  data) {
                setPosition(fieldName);
                dataOutput.writeLongArray(data);
            };

            void PortableWriter::writeFloatArray(const char *fieldName, const std::vector<float >&  data) {
                setPosition(fieldName);
                dataOutput.writeFloatArray(data);
            };

            void PortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double >&  data) {
                setPosition(fieldName);
                dataOutput.writeDoubleArray(data);
            };

            void PortableWriter::setPosition(const char *fieldName) {
                if (raw) throw exception::IOException("PortableWriter::setPosition", "Cannot write Portable fields after getRawDataOutput() is called!");
                if (!cd->isFieldDefinitionExists(fieldName)) {
                    std::string error;
                    error += "HazelcastSerializationException( Invalid field name: '";
                    error += fieldName;
                    error += "' for ClassDefinition {id: "; //TODO add factory ID
                    error += hazelcast::util::to_string(cd->getClassId());
                    error += ", version: ";
                    error += hazelcast::util::to_string(cd->getVersion());
                    error += "}";

                    throw exception::IOException("PortableWriter::setPosition", error);
                }

                if (writtenFields.count(fieldName) != 0)
                    throw exception::IOException("PortableWriter::setPosition", "Field '" + std::string(fieldName) + "' has already been written!");
                writtenFields.insert(fieldName);
                dataOutput.writeInt(offset + cd->get(fieldName).getIndex() * sizeof (int), dataOutput.position());

            };

            ObjectDataOutput *PortableWriter::getRawDataOutput() {
                if (!raw) {
                    int pos = dataOutput.position();
                    int index = cd->getFieldCount(); // last index
                    dataOutput.writeInt(offset + index * sizeof(int), pos);
                }
                raw = true;
                return &dataOutput; //TODO why return pointer not reference
            };

            void PortableWriter::end() {
                dataOutput.writeInt(begin, dataOutput.position()); // write final offset
            };


        }
    }
}