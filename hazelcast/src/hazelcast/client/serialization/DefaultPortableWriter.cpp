//
//  PortableWriter.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "DefaultPortableWriter.h"
#include "ClassDefinition.h"
#include "SerializationContext.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            DefaultPortableWriter::DefaultPortableWriter(SerializationContext& serializationContext, util::AtomicPointer<ClassDefinition> cd, DataOutput& dataOutput)
            : context(serializationContext)
            , serializerHolder(serializationContext.getSerializerHolder())
            , dataOutput(dataOutput)
            , objectDataOutput(dataOutput, serializationContext)
            , index (0)
            , cd(cd)
            , raw(false)
            , begin(dataOutput.position())
            , offset(dataOutput.position() + sizeof(int)) {
                int const fieldIndexesLength = (cd->getFieldCount() + 1) * sizeof (int);
                this->dataOutput.position(offset + fieldIndexesLength);
            };

            void DefaultPortableWriter::writeInt(const char *fieldName, int value) {
                setPosition(fieldName);
                dataOutput.writeInt(value);
            };

            void DefaultPortableWriter::writeLong(const char *fieldName, long value) {
                setPosition(fieldName);
                dataOutput.writeLong(value);
            };

            void DefaultPortableWriter::writeBoolean(const char *fieldName, bool value) {
                setPosition(fieldName);
                dataOutput.writeBoolean(value);
            };

            void DefaultPortableWriter::writeByte(const char *fieldName, byte value) {
                setPosition(fieldName);
                dataOutput.writeByte(value);
            };

            void DefaultPortableWriter::writeChar(const char *fieldName, int value) {
                setPosition(fieldName);
                dataOutput.writeChar(value);
            };

            void DefaultPortableWriter::writeDouble(const char *fieldName, double value) {
                setPosition(fieldName);
                dataOutput.writeDouble(value);
            };

            void DefaultPortableWriter::writeFloat(const char *fieldName, float value) {
                setPosition(fieldName);
                dataOutput.writeFloat(value);
            };

            void DefaultPortableWriter::writeShort(const char *fieldName, short value) {
                setPosition(fieldName);
                dataOutput.writeShort(value);
            };

            void DefaultPortableWriter::writeUTF(const char *fieldName, const string& value) {
                setPosition(fieldName);
                dataOutput.writeUTF(value);
            };

            void DefaultPortableWriter::writeNullPortable(const char *fieldName, int factoryId, int classId) {
                setPosition(fieldName);
                dataOutput.writeBoolean(true);
            };

            void DefaultPortableWriter::writeByteArray(const char *fieldName, const vector<byte>  & bytes) {
                setPosition(fieldName);
                dataOutput.writeByteArray(bytes);
            };

            void DefaultPortableWriter::writeCharArray(const char *fieldName, const std::vector<char >&  data) {
                setPosition(fieldName);
                dataOutput.writeCharArray(data);
            };

            void DefaultPortableWriter::writeShortArray(const char *fieldName, const std::vector<short >&  data) {
                setPosition(fieldName);
                dataOutput.writeShortArray(data);
            };

            void DefaultPortableWriter::writeIntArray(const char *fieldName, const std::vector<int>&  data) {
                setPosition(fieldName);
                dataOutput.writeIntArray(data);
            };

            void DefaultPortableWriter::writeLongArray(const char *fieldName, const std::vector<long >&  data) {
                setPosition(fieldName);
                dataOutput.writeLongArray(data);
            };

            void DefaultPortableWriter::writeFloatArray(const char *fieldName, const std::vector<float >&  data) {
                setPosition(fieldName);
                dataOutput.writeFloatArray(data);
            };

            void DefaultPortableWriter::writeDoubleArray(const char *fieldName, const std::vector<double >&  data) {
                setPosition(fieldName);
                dataOutput.writeDoubleArray(data);
            };

            void DefaultPortableWriter::setPosition(const char *fieldName) {
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


            ObjectDataOutput& DefaultPortableWriter::getRawDataOutput() {
                if (!raw) {
                    int pos = dataOutput.position();
                    int index = cd->getFieldCount(); // last index
                    dataOutput.writeInt(offset + index * sizeof(int), pos);
                }
                raw = true;
                return objectDataOutput;
            };

            void DefaultPortableWriter::end() {
                dataOutput.writeInt(begin, dataOutput.position()); // write final offset
            };

            util::AtomicPointer<ClassDefinition> DefaultPortableWriter::getClassDefinition(const Portable& p) {
                return serializerHolder.getPortableSerializer().getClassDefinition(p);
            };

            void DefaultPortableWriter::write(const Portable& p) {
                return serializerHolder.getPortableSerializer().write(dataOutput, p);
            }


        }
    }
}