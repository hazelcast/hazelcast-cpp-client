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
#include "../Array.h"
#include <iostream>

using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{

class ConstantSerializers {
public:
    
    class ByteSerializer : public TypeSerializer<byte> {
    public:
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BYTE;
        };
        
        byte read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readByte();
        };
        
        void write(DataOutput* dataOutput, byte const obj) throw(ios_base::failure){
            dataOutput->writeByte(obj);
        };
    };
    
    class BooleanSerializer : public TypeSerializer<bool> {
    public:
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
        };
        
        void write(DataOutput* dataOutput, bool obj) throw(ios_base::failure){
            dataOutput->writeBoolean(obj);
        };
        
        bool read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readBoolean();
        }
    };
    
    class CharSerializer : public TypeSerializer<char> {
    public:
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_CHAR;
        }
        
        char read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readChar();
        }
        
        void write(DataOutput* dataOutput, const char obj) throw(ios_base::failure){
            dataOutput->writeChar(obj);
        }
    };
    
    class ShortSerializer : public TypeSerializer<short> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_SHORT;
        }
        
        short read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readShort();
        }
        
        void write(DataOutput* dataOutput, const short obj) throw(ios_base::failure){
            dataOutput->writeShort(obj);
        }
    };
    
    class IntegerSerializer : public TypeSerializer<int> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_INTEGER;
        }
        
        int read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readInt();
        }
        
        void write(DataOutput* dataOutput, const int obj) throw(ios_base::failure){
            dataOutput->writeInt(obj);
        }
    };
    
    class LongSerializer : public TypeSerializer<long> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_LONG;
        }
        
        long read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readLong();
        }
        
        void write(DataOutput* dataOutput, const long obj) throw(ios_base::failure){
            dataOutput->writeLong(obj);
        }
    };
    
    class FloatSerializer : public TypeSerializer<float> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_FLOAT;
        }
        
        float read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readFloat();
        }
        
        void write(DataOutput* dataOutput, const float obj) throw(ios_base::failure){
            dataOutput->writeFloat(obj);
        }
    };
    
    class DoubleSerializer : public TypeSerializer<double> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_DOUBLE;
        }
        
        double read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readDouble();
        }
        
        void write(DataOutput* dataOutput, const double obj) throw(ios_base::failure){
            dataOutput->writeDouble(obj);
        }
    };
    
    class StringSerializer : public TypeSerializer<string> {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_STRING;
        }
        
        string read(DataInput* dataInput) throw(ios_base::failure){
            return dataInput->readUTF();
        }
        
        void write(DataOutput* dataOutput, const string obj) throw(ios_base::failure){
            dataOutput->writeUTF(obj);
        }
    };
    
    class ByteArraySerializer : public TypeSerializer<Array<byte> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
        }
        
        Array<byte> read(DataInput* dataInput) throw(ios_base::failure){
            int size = dataInput->readInt();
            Array<byte> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readByte();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, Array<byte>& obj) throw(ios_base::failure){
            int size = obj.length();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeByte(obj[i]);
                }
            }
        }
    };
    
    class CharArraySerializer : public TypeSerializer<Array<char> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
        }
        
        Array<char> read(DataInput* dataInput) throw(ios_base::failure){
            int size = dataInput->readInt();
            Array<char> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readChar();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, Array<char>& obj) throw(ios_base::failure){
            int size = obj.length();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeChar(obj[i]);
                }
            }
        }
    };
    
    class ShortArraySerializer : public TypeSerializer<Array<short> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
        }
        
        Array<short> read(DataInput* dataInput) throw(ios_base::failure){
            int size = dataInput->readInt();
            Array<short> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readShort();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, Array<short>& obj) throw(ios_base::failure){
            int size = obj.length();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeShort(obj[i]);
                }
            }
        }
    };
    
    class IntegerArraySerializer : public TypeSerializer<Array<int> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
        }
        
        Array<int> read(DataInput* dataInput) throw(ios_base::failure){
             int size = dataInput->readInt();
            Array<int> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readInt();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, Array<int>& obj) throw(ios_base::failure){
            int size = obj.length();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeInt(obj[i]);
                }
            }
        }
    };
    
    class LongArraySerializer : public TypeSerializer<Array<long> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
        }
        
        Array<long> read(DataInput* dataInput) throw(ios_base::failure){
             int size = dataInput->readInt();
            Array<long> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readLong();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, Array<long>& obj) throw(ios_base::failure){
            int size = obj.length();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeLong(obj[i]);
                }
            }
        }
    };
    
    class FloatArraySerializer : public TypeSerializer<Array<float> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
        }
        
        Array<float> read(DataInput* dataInput) throw(ios_base::failure){
             int size = dataInput->readInt();
            Array<float> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readShort();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, Array<float>& obj) throw(ios_base::failure){
            int size = obj.length();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeFloat(obj[i]);
                }
            }
        }
    };
    
    class DoubleArraySerializer : public TypeSerializer<Array<double> > {
    public:
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
        }
        
        Array<double> read(DataInput* dataInput) throw(ios_base::failure){
             int size = dataInput->readInt();
            Array<double> c(size);
            for (int i = 0; i < size; i++) {
                c[i] = dataInput->readShort();
            }
            return c;
        }
        
        void write(DataOutput* dataOutput, Array<double>& obj) throw(ios_base::failure){
            int size = obj.length();
            dataOutput->writeInt(size);
            if (size > 0) {
                for(int i = 0; i < size ; i++){
                    dataOutput->writeDouble(obj[i]);
                }
            }
        }
    };
     
private:
    
    
        ConstantSerializers();
};

}}}
#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
