//
//  ObjectDataInput.cpp
//  Server
//
//  Created by sancar koyunlu on 1/3/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/ObjectDataInput.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ObjectDataInput::ObjectDataInput(pimpl::DataInput& dataInput, pimpl::PortableContext& context)
            : dataInput(dataInput)
            , portableContext(context)
            , serializerHolder(context.getSerializerHolder()) {

            }

            pimpl::PortableContext *ObjectDataInput::getPortableContext() {
                return &portableContext;
            }

            void ObjectDataInput::readFully(std::vector<byte>& bytes) {
                dataInput.readFully(bytes);
            }

            int ObjectDataInput::skipBytes(int i) {
                return dataInput.skipBytes(i);
            }

            bool ObjectDataInput::readBoolean() {
                return dataInput.readBoolean();
            }

            byte ObjectDataInput::readByte() {
                return dataInput.readByte();
            }

            short ObjectDataInput::readShort() {
                return dataInput.readShort();
            }

            char ObjectDataInput::readChar() {
                return dataInput.readChar();
            }

            int ObjectDataInput::readInt() {
                return dataInput.readInt();
            }

            long ObjectDataInput::readLong() {
                return (long)dataInput.readLong();
            }

            float ObjectDataInput::readFloat() {
                return dataInput.readFloat();
            }

            double ObjectDataInput::readDouble() {
                return dataInput.readDouble();
            }

            std::auto_ptr<std::string> ObjectDataInput::readUTF() {
                return dataInput.readUTF();
            }

            pimpl::Data ObjectDataInput::readData() {
                return pimpl::Data(dataInput.readByteArray());
            }

            int ObjectDataInput::position() {
                return dataInput.position();
            }

            void ObjectDataInput::position(int newPos) {
                dataInput.position(newPos);
            }

            std::auto_ptr<std::vector<byte> > ObjectDataInput::readByteArray() {
                return dataInput.readByteArray();
            }

            std::auto_ptr<std::vector<bool> > ObjectDataInput::readBooleanArray() {
                return dataInput.readBooleanArray();
            }

            std::auto_ptr<std::vector<char> > ObjectDataInput::readCharArray() {
                return dataInput.readCharArray();
            }

            std::auto_ptr<std::vector<int> > ObjectDataInput::readIntArray() {
                return dataInput.readIntArray();
            }

            std::auto_ptr<std::vector<long> > ObjectDataInput::readLongArray() {
                return dataInput.readLongArray();
            }

            std::auto_ptr<std::vector<double> > ObjectDataInput::readDoubleArray() {
                return dataInput.readDoubleArray();
            }

            std::auto_ptr<std::vector<float> > ObjectDataInput::readFloatArray() {
                return dataInput.readFloatArray();
            }

            std::auto_ptr<std::vector<short> > ObjectDataInput::readShortArray() {
                return dataInput.readShortArray();
            }

            std::auto_ptr<common::containers::ManagedPointerVector<std::string> > ObjectDataInput::readUTFArray() {
                return dataInput.readUTFArray();
            }

            void ObjectDataInput::readPortable(Portable * object) {
                ObjectDataInput in(dataInput, portableContext);
                
                int factoryId = readInt();
                int classId = readInt();
                serializerHolder.getPortableSerializer().read(dataInput, *object, factoryId, classId);
            }

            void ObjectDataInput::readDataSerializable(IdentifiedDataSerializable * object) {
                ObjectDataInput input(dataInput, portableContext);
                serializerHolder.getDataSerializer().read(input, *object);
            }
        }
    }
}

