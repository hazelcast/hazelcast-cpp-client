//
//  ConstantSerializers.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CONSTANT_SERIALIZERS
#define HAZELCAST_CONSTANT_SERIALIZERS

#include "DataInput.h"
#include "DataOutput.h"
#include "SerializationConstants.h"
#include "TypeSerializer.h"

#include <iostream>

using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{

namespace ConstantSerializers{
    
    class ByteSerializer : public TypeSerializer<byte> {
    public:
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BYTE;
        };
        
        byte read(DataInput* dataInput){
            return dataInput->readByte();
        };
        
        void write(DataOutput* dataOutput, byte const obj){
            dataOutput->writeByte(obj);
        };
    };
    
    class BooleanSerializer : public TypeSerializer<bool> {
    public:
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
        };
        
        void write(DataOutput* dataOutput, bool obj){
            dataOutput->writeBoolean(obj);
        };
        
        bool read(DataInput* dataInput){
            return dataInput->readBoolean();
        }
    };
    
    class CharSerializer : public TypeSerializer<char> {
    public:
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_CHAR;
        }
        
        char read(DataInput* dataInput){
            return dataInput->readChar();
        }
        
        void write(DataOutput* dataOutput, const char obj){
            dataOutput->writeChar(obj);
        }
    };
    
    class ShortSerializer : public TypeSerializer<short> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_SHORT;
        }
        
        short read(DataInput* dataInput){
            return dataInput->readShort();
        }
        
        void write(DataOutput* dataOutput, const short obj){
            dataOutput->writeShort(obj);
        }
    };
    
    class IntegerSerializer : public TypeSerializer<int> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_INTEGER;
        }
        
        int read(DataInput* dataInput){
            return dataInput->readInt();
        }
        
        void write(DataOutput* dataOutput, const int obj){
            dataOutput->writeInt(obj);
        }
    };
    
    class LongSerializer : public TypeSerializer<long> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_LONG;
        }
        
        long read(DataInput* dataInput){
            return dataInput->readLong();
        }
        
        void write(DataOutput* dataOutput, const long obj){
            dataOutput->writeLong(obj);
        }
    };
    
    class FloatSerializer : public TypeSerializer<float> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_FLOAT;
        }
        
        float read(DataInput* dataInput){
            return dataInput->readFloat();
        }
        
        void write(DataOutput* dataOutput, const float obj){
            dataOutput->writeFloat(obj);
        }
    };
    
    class DoubleSerializer : public TypeSerializer<double> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_DOUBLE;
        }
        
        double read(DataInput* dataInput){
            return dataInput->readDouble();
        }
        
        void write(DataOutput* dataOutput, const double obj){
            dataOutput->writeDouble(obj);
        }
    };
    
    class StringSerializer : public TypeSerializer<string> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_STRING;
        }
        
        string read(DataInput* dataInput){
            return dataInput->readUTF();
        }
        
        void write(DataOutput* dataOutput, const string obj){
            dataOutput->writeUTF(obj);
        }
    };
    
    class ByteArraySerializer : public TypeSerializer<std::vector<byte> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
        }
        
        std::vector<byte> read(DataInput* dataInput){
            int size = dataInput->readInt();
            std::vector<byte> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readByte();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, std::vector<byte>& obj){
            int size = obj.size();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeByte(obj[i]);
                }
            }
        }
    };
    
    class CharArraySerializer : public TypeSerializer<std::vector<char> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
        }
        
        std::vector<char> read(DataInput* dataInput){
            int size = dataInput->readInt();
            std::vector<char> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readChar();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, std::vector<char>& obj){
            int size = obj.size();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeChar(obj[i]);
                }
            }
        }
    };
    
    class ShortArraySerializer : public TypeSerializer<std::vector<short> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
        }
        
        std::vector<short> read(DataInput* dataInput){
            int size = dataInput->readInt();
            std::vector<short> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readShort();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, std::vector<short>& obj){
            int size = obj.size();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeShort(obj[i]);
                }
            }
        }
    };
    
    class IntegerArraySerializer : public TypeSerializer<std::vector<int> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
        }
        
        std::vector<int> read(DataInput* dataInput){
             int size = dataInput->readInt();
            std::vector<int> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readInt();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, std::vector<int>& obj){
            int size = obj.size();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeInt(obj[i]);
                }
            }
        }
    };
    
    class LongArraySerializer : public TypeSerializer<std::vector<long> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
        }
        
        std::vector<long> read(DataInput* dataInput){
             int size = dataInput->readInt();
            std::vector<long> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readLong();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, std::vector<long>& obj){
            int size = obj.size();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeLong(obj[i]);
                }
            }
        }
    };
    
    class FloatArraySerializer : public TypeSerializer<std::vector<float> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
        }
        
        std::vector<float> read(DataInput* dataInput){
             int size = dataInput->readInt();
            std::vector<float> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readShort();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, std::vector<float>& obj){
            int size = obj.size();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeFloat(obj[i]);
                }
            }
        }
    };
    
    class DoubleArraySerializer : public TypeSerializer<std::vector<double> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
        }
        
        std::vector<double> read(DataInput* dataInput){
             int size = dataInput->readInt();
            std::vector<double> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readShort();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, std::vector<double>& obj){
            int size = obj.size();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeDouble(obj[i]);
                }
            }
        }
    };

}

}}}
#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
