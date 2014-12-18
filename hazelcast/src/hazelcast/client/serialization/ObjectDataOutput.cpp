//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            ObjectDataOutput::ObjectDataOutput(pimpl::DataOutput& dataOutput, pimpl::PortableContext& portableContext)
            : dataOutput(&dataOutput)
            , context(&portableContext)
            , serializerHolder(&portableContext.getSerializerHolder())
            , isEmpty(false) {

            }

            ObjectDataOutput::ObjectDataOutput()
            : dataOutput(NULL)
            , context(NULL)
            , serializerHolder(NULL)
            , isEmpty(true) {

            }

            std::auto_ptr<std::vector<byte> > ObjectDataOutput::toByteArray() {
                if (isEmpty)
                    return std::auto_ptr<std::vector<byte> >(NULL);
                return dataOutput->toByteArray();
            }

            void ObjectDataOutput::write(const std::vector<byte>& bytes) {
                if (isEmpty) return;
                dataOutput->write(bytes);
            }

            void ObjectDataOutput::writeBoolean(bool i) {
                if (isEmpty) return;
                dataOutput->writeBoolean(i);
            }

            void ObjectDataOutput::writeByte(int i) {
                if (isEmpty) return;
                dataOutput->writeByte(i);
            }

            void ObjectDataOutput::writeShort(int v) {
                if (isEmpty) return;
                dataOutput->writeShort(v);
            }

            void ObjectDataOutput::writeChar(int i) {
                if (isEmpty) return;
                dataOutput->writeChar(i);
            }

            void ObjectDataOutput::writeInt(int v) {
                if (isEmpty) return;
                dataOutput->writeInt(v);
            }

            void ObjectDataOutput::writeLong(long l) {
                if (isEmpty) return;
                dataOutput->writeLong(l);
            }

            void ObjectDataOutput::writeFloat(float x) {
                if (isEmpty) return;
                dataOutput->writeFloat(x);
            }

            void ObjectDataOutput::writeDouble(double v) {
                if (isEmpty) return;
                dataOutput->writeLong((long)v);
            }

            void ObjectDataOutput::writeUTF(const std::string& str) {
                if (isEmpty) return;
                dataOutput->writeUTF(str);
            }

            void ObjectDataOutput::writeByteArray(const std::vector<byte>& data) {
                if (isEmpty) return;
                dataOutput->writeByteArray(data);
            }

            void ObjectDataOutput::writeCharArray(const std::vector<char>& data) {
                if (isEmpty) return;
                dataOutput->writeCharArray(data);
            }

            void ObjectDataOutput::writeShortArray(const std::vector<short>& data) {
                if (isEmpty) return;
                dataOutput->writeShortArray(data);
            }

            void ObjectDataOutput::writeIntArray(const std::vector<int>& data) {
                if (isEmpty) return;
                dataOutput->writeIntArray(data);
            }

            void ObjectDataOutput::writeLongArray(const std::vector<long>& data) {
                if (isEmpty) return;
                dataOutput->writeLongArray(data);
            }

            void ObjectDataOutput::writeFloatArray(const std::vector<float>& data) {
                if (isEmpty) return;
                dataOutput->writeFloatArray(data);
            }

            void ObjectDataOutput::writeDoubleArray(const std::vector<double>& data) {
                if (isEmpty) return;
                dataOutput->writeDoubleArray(data);
            }


            void ObjectDataOutput::writeData(const pimpl::Data *data) {
                if (isEmpty) return;
                bool isNull = data == NULL;
                writeBoolean(isNull);
                if (isNull) {
                    return;
                }
                writeInt(data->getType());
                writeInt(data->hasPartitionHash() ? data->getPartitionHash() : 0);
                writePortableHeader(*data);

                int size = data->bufferSize();
                writeInt(size);
                if (size > 0) {
                    dataOutput->write(*(data->data));
                }
            }

            void ObjectDataOutput::writePortableHeader(const pimpl::Data& data) {
                if (!data.hasClassDefinition()) {
                    writeInt(0);
                } else {
                    util::ByteBuffer& headerBuffer = dataOutput->getHeaderBuffer();
                    writeInt(data.header->size());
                    writeInt(headerBuffer.position());
                    headerBuffer.readFrom(*data.header);
                }
            }


            void ObjectDataOutput::writePortable(const Portable *portable) {
                writeBoolean(false);
                writeInt(portable->getSerializerId());
                serializerHolder->getPortableSerializer().write(*dataOutput, *portable);

            }

            void ObjectDataOutput::writeIdentifiedDataSerializable(const IdentifiedDataSerializable *dataSerializable) {
                writeBoolean(false);
                writeInt(dataSerializable->getSerializerId());
                serializerHolder->getDataSerializer().write(*this, *dataSerializable);
            }


            int ObjectDataOutput::position() {
                return dataOutput->position();
            }

            void ObjectDataOutput::position(int newPos) {
                dataOutput->position(newPos);
            }

        }
    }
}

