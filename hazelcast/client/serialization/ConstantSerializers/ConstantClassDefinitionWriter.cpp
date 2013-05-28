//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "ConstantClassDefinitionWriter.h"
#include "ClassDefinitionWriter.h"
#include "NullPortable.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            void writePortable(ClassDefinitionWriter& cdw, byte data) {
                cdw.writeByte(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, bool data) {
                cdw.writeBoolean(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, char data) {
                cdw.writeChar(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, short data) {
                cdw.writeShort(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, int data) {
                cdw.writeInt(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, long data) {
                cdw.writeLong(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, float data) {
                cdw.writeFloat(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, double data) {
                cdw.writeDouble(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::string&   data) {
                cdw.writeUTF(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<byte>&  data) {
                cdw.writeByteArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<char >&  data) {
                cdw.writeCharArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<short >&  data) {
                cdw.writeShortArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<int>&  data) {
                cdw.writeIntArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<long >&  data) {
                cdw.writeLongArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<float >&  data) {
                cdw.writeFloatArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const std::vector<double >&  data) {
                cdw.writeDoubleArray(data);
            };

            void writePortable(ClassDefinitionWriter& cdw, const NullPortable&  data) {
                cdw.writeNullPortable(data.factoryId, data.classId);
            };

            void writePortable(ClassDefinitionWriter& cdw, const Data&  data) {

            };

            void operator <<(ClassDefinitionWriter& cdw, byte data) {
                cdw.writeByte(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, bool data) {
                cdw.writeBoolean(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, char data) {
                cdw.writeChar(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, short data) {
                cdw.writeShort(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, int data) {
                cdw.writeInt(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, long data) {
                cdw.writeLong(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, float data) {
                cdw.writeFloat(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, double data) {
                cdw.writeDouble(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::string&   data) {
                cdw.writeUTF(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::vector<byte>&  data) {
                cdw.writeByteArray(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::vector<char >&  data) {
                cdw.writeCharArray(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::vector<short >&  data) {
                cdw.writeShortArray(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::vector<int>&  data) {
                cdw.writeIntArray(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::vector<long >&  data) {
                cdw.writeLongArray(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::vector<float >&  data) {
                cdw.writeFloatArray(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const std::vector<double >&  data) {
                cdw.writeDoubleArray(data);
            };

            void operator <<(ClassDefinitionWriter& cdw, const NullPortable&  data) {
                cdw.writeNullPortable(data.factoryId, data.classId);
            };

            void operator <<(ClassDefinitionWriter& cdw, const Data&  data) {

            };


        }
    }
}
