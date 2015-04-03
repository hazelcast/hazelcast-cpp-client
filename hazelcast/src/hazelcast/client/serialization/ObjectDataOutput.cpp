//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/serialization/pimpl/ClassDefinitionWriter.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            ObjectDataOutput::ObjectDataOutput(pimpl::DataOutput& dataOutput,
                    pimpl::PortableContext& portableContext)
            : dataOutput(&dataOutput)
            , context(&portableContext)
            , serializerHolder(&portableContext.getSerializerHolder()),
              serializationSrv(&portableContext.getSerializationService())
            , isEmpty(false) {

            }

            ObjectDataOutput::ObjectDataOutput()
            : dataOutput(NULL)
            , context(NULL)
            , serializerHolder(NULL)
            , isEmpty(true) {

            }

            boost::shared_ptr<std::vector<byte> > ObjectDataOutput::toByteArray() {
                if (isEmpty)
                    return boost::shared_ptr<std::vector<byte> >((std::vector<byte> *)0);
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
                dataOutput->writeByte((byte) i);
            }

            void ObjectDataOutput::writeShort(int v) {
                if (isEmpty) return;
                dataOutput->writeShort((short) v);
            }

            void ObjectDataOutput::writeChar(int i) {
                if (isEmpty) return;
                dataOutput->writeChar((short) i);
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
                bool isNull = (0 == data || 0 == data->totalSize());
                writeBoolean(isNull);
                if (isNull) {
                    return;
                }
                writeByteArray(data->toByteArray());
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

            size_t ObjectDataOutput::position() {
                return dataOutput->position();
            }

            void ObjectDataOutput::position(size_t newPos) {
                dataOutput->position(newPos);
            }

            void ObjectDataOutput::writeZeroBytes(int numberOfBytes) {
                for (int k = 0; k < numberOfBytes; k++) {
                    writeByte(0);
                }
            }

            template<typename T>
            void ObjectDataOutput::writeObject(const Portable *object) {
                serializationSrv->writeObject(*dataOutput, object);
            }

            template<typename T>
            void ObjectDataOutput::writeObject(const IdentifiedDataSerializable *object) {
                serializationSrv->writeObject(*dataOutput, object);
            }

            template<typename T>
            void ObjectDataOutput::writeObject(const void *object) {
                serializationSrv->writeObject<T>(*dataOutput, object);
            }
        }
    }
}

