//
// Created by sancar koyunlu on 8/12/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ClassDefinitionWriter.h"
#include "hazelcast/client/IdentifiedDataSerializable.h"
#include "SerializationContext.h"
#include "hazelcast/client/Portable.h"
#include "DataOutput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            ObjectDataOutput::ObjectDataOutput(DataOutput& dataOutput, SerializationContext& serializationContext)
            : context(&serializationContext)
            , serializerHolder(&serializationContext.getSerializerHolder())
            , isEmpty(false)
            , dataOutput(&dataOutput) {

            };

            ObjectDataOutput::ObjectDataOutput()
            : serializerHolder(NULL)
            , context(NULL)
            , dataOutput(NULL)
            , isEmpty(true) {

            };


            ObjectDataOutput::ObjectDataOutput(ObjectDataOutput const & param) {
                //private
            }

            void ObjectDataOutput::operator = (ObjectDataOutput const & param) {
                //private
            }

            std::auto_ptr< std::vector<byte> > ObjectDataOutput::toByteArray() {
                if (isEmpty)
                    return std::auto_ptr< std::vector<byte > >(NULL);
                return dataOutput->toByteArray();
            };

            void ObjectDataOutput::write(const std::vector<byte>& bytes) {
                if (isEmpty) return;
                dataOutput->write(bytes);
            };

            void ObjectDataOutput::writeBoolean(bool i) {
                if (isEmpty) return;
                dataOutput->writeBoolean(i);
            };

            void ObjectDataOutput::writeByte(int index, int i) {
                if (isEmpty) return;
                dataOutput->writeByte(index, i);
            }

            void ObjectDataOutput::writeByte(int i) {
                if (isEmpty) return;
                dataOutput->writeByte(i);
            };

            void ObjectDataOutput::writeShort(int v) {
                if (isEmpty) return;
                dataOutput->writeShort(v);
            };

            void ObjectDataOutput::writeChar(int i) {
                if (isEmpty) return;
                dataOutput->writeChar(i);
            };

            void ObjectDataOutput::writeInt(int v) {
                if (isEmpty) return;
                dataOutput->writeInt(v);
            };

            void ObjectDataOutput::writeLong(long l) {
                if (isEmpty) return;
                dataOutput->writeLong(l);
            };

            void ObjectDataOutput::writeFloat(float x) {
                if (isEmpty) return;
                dataOutput->writeFloat(x);
            };

            void ObjectDataOutput::writeDouble(double v) {
                if (isEmpty) return;
                dataOutput->writeLong(v);
            };

            void ObjectDataOutput::writeUTF(const std::string& str) {
                if (isEmpty) return;
                dataOutput->writeUTF(str);
            };

            void ObjectDataOutput::writeInt(int index, int v) {
                if (isEmpty) return;
                dataOutput->writeInt(index, v);
            };

            void ObjectDataOutput::writeByteArray(const std::vector<byte>&  data) {
                if (isEmpty) return;
                dataOutput->writeByteArray(data);
            };

            void ObjectDataOutput::writeCharArray(const std::vector<char>& data) {
                if (isEmpty) return;
                dataOutput->writeCharArray(data);
            };

            void ObjectDataOutput::writeShortArray(const std::vector<short >&  data) {
                if (isEmpty) return;
                dataOutput->writeShortArray(data);
            };

            void ObjectDataOutput::writeIntArray(const std::vector<int>&  data) {
                if (isEmpty) return;
                dataOutput->writeIntArray(data);
            };

            void ObjectDataOutput::writeLongArray(const std::vector<long >&  data) {
                if (isEmpty) return;
                dataOutput->writeLongArray(data);
            };

            void ObjectDataOutput::writeFloatArray(const std::vector<float >&  data) {
                if (isEmpty) return;
                dataOutput->writeFloatArray(data);
            };

            void ObjectDataOutput::writeDoubleArray(const std::vector<double >&  data) {
                if (isEmpty) return;
                dataOutput->writeDoubleArray(data);
            };


            void ObjectDataOutput::writeNullObject() {
                if (isEmpty) return;
                writeBoolean(true);
            };

            void ObjectDataOutput::writePortable(const Portable *portable) {
                writeBoolean(false);
                writeInt(portable->getSerializerId());
                ClassDefinition *cd = context->lookup(portable->getFactoryId(), portable->getClassId());
                if (cd == NULL) {
                    ClassDefinitionWriter classDefinitionWriter(portable->getFactoryId(), portable->getClassId(), context->getVersion(), *context);
                    cd = classDefinitionWriter.getOrBuildClassDefinition(*portable);
                    cd->writeData(*dataOutput);
                }
                serializerHolder->getPortableSerializer().write(*dataOutput, *portable);

            };

            void ObjectDataOutput::writeIdentifiedDataSerializable(const IdentifiedDataSerializable *dataSerializable) {
                writeBoolean(false);
                writeInt(dataSerializable->getSerializerId());
                serializerHolder->getDataSerializer().write(*this, *dataSerializable);
            };


            int ObjectDataOutput::position() {
                return dataOutput->position();
            };

            void ObjectDataOutput::position(int newPos) {
                dataOutput->position(newPos);
            };

            void ObjectDataOutput::reset() {
                dataOutput->reset();
            };
        }
    }
}
