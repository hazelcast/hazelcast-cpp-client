//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/util/Bits.h"
#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            ObjectDataOutput::ObjectDataOutput(pimpl::DataOutput& dataOutput,
                    pimpl::PortableContext& portableContext)
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
                    return std::auto_ptr<std::vector<byte> >((std::vector<byte> *)NULL);
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

            void ObjectDataOutput::writeUTF(const std::string *str) {
                if (isEmpty) return;

                if (NULL == str) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    dataOutput->writeUTF(str);
                }
            }

            void ObjectDataOutput::writeByteArray(const std::vector<byte> *value) {
                if (isEmpty) return;
                dataOutput->writeByteArray(value);
            }

            void ObjectDataOutput::writeCharArray(const std::vector<char> *data) {
                if (isEmpty) return;
                dataOutput->writeCharArray(data);
            }

            void ObjectDataOutput::writeShortArray(const std::vector<short> *data) {
                if (isEmpty) return;
                dataOutput->writeShortArray(data);
            }

            void ObjectDataOutput::writeIntArray(const std::vector<int> *data) {
                if (isEmpty) return;
                dataOutput->writeIntArray(data);
            }

            void ObjectDataOutput::writeLongArray(const std::vector<long> *data) {
                if (isEmpty) return;
                dataOutput->writeLongArray(data);
            }

            void ObjectDataOutput::writeFloatArray(const std::vector<float> *data) {
                if (isEmpty) return;
                dataOutput->writeFloatArray(data);
            }

            void ObjectDataOutput::writeDoubleArray(const std::vector<double> *data) {
                if (isEmpty) return;
                dataOutput->writeDoubleArray(data);
            }

            void ObjectDataOutput::writeUTFArray(const std::vector<std::string *> *strings) {
                if (isEmpty) return;

                int len = NULL != strings ? (int)strings->size() : util::Bits::NULL_ARRAY;

                writeInt(len);

                if (len > 0) {
                    for (std::vector<std::string *>::const_iterator it = strings->begin(); it != strings->end(); ++it) {
                        writeUTF(*it);
                    }
                }
            }

            void ObjectDataOutput::writeData(const pimpl::Data *data) {
                if (NULL == data) {
                    writeInt(util::Bits::NULL_ARRAY);
                } else {
                    writeByteArray(&data->toByteArray());
                }

            }

            size_t ObjectDataOutput::position() {
                return dataOutput->position();
            }

            void ObjectDataOutput::position(size_t newPos) {
                dataOutput->position(newPos);
            }
        }
    }
}

