//
//  ConstantSerializers.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_ConstantSerializers_h
#define Server_ConstantSerializers_h

using namespace std;
#include "DataInput.h"
#include "DataOutput.h"
#include "SerializationConstants.h"
class ConstantSerializers {
public:
    
    class ByteSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BYTE;
        };
        
        byte read(DataInput input) throw(ios_base::failure){
            return input.readByte();
        };
        
        void write(DataOutput output, byte const obj) throw(ios_base::failure){
            output.writeByte(obj);
        };
    };
    
    class BooleanSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
        };
        
        void write(DataOutput output, bool obj) throw(ios_base::failure){
//            output.write((obj ? 1 : 0));
        };
        
        bool read(DataInput input) throw(ios_base::failure){
            return input.readByte() != 0;
        }
    };
    
    class CharSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_CHAR;
        }
        
        char read(DataInput input) throw(ios_base::failure){
            return input.readChar();
        }
        
        void write(DataOutput output, const char obj) throw(ios_base::failure){
            output.writeChar(obj);
        }
    };
    
    class ShortSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_SHORT;
        }
        
        short read(DataInput input) throw(ios_base::failure){
            return input.readShort();
        }
        
        void write(DataOutput output, const short obj) throw(ios_base::failure){
            output.writeShort(obj);
        }
    };
    
    class IntegerSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_INTEGER;
        }
        
        int read(DataInput input) throw(ios_base::failure){
            return input.readInt();
        }
        
        void write(DataOutput output, const int obj) throw(ios_base::failure){
            output.writeInt(obj);
        }
    };
    
    class LongSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_LONG;
        }
        
        long read(DataInput input) throw(ios_base::failure){
            return input.readLong();
        }
        
        void write(DataOutput output, const long obj) throw(ios_base::failure){
            output.writeLong(obj);
        }
    };
    
    class FloatSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_FLOAT;
        }
        
        float read(DataInput input) throw(ios_base::failure){
            return input.readFloat();
        }
        
        void write(DataOutput output, const float obj) throw(ios_base::failure){
            output.writeFloat(obj);
        }
    };
    
    class DoubleSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_DOUBLE;
        }
        
        double read(DataInput input) throw(ios_base::failure){
            return input.readDouble();
        }
        
        void write(DataOutput output, const double obj) throw(ios_base::failure){
            output.writeDouble(obj);
        }
    };
    
    class StringSerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_STRING;
        }
        
        string read(DataInput input) throw(ios_base::failure){
            return input.readUTF();
        }
        
        void write(DataOutput output, const string obj) throw(ios_base::failure){
            output.writeUTF(obj);
        }
    };
    
    class ByteArraySerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
        }
        
        byte* read(DataInput input) throw(ios_base::failure){
            int size = input.readInt();
            if (size == 0) {
                return NULL;
            } else {
                byte* b = new byte[size];
                input.readFully(b,0,size);
                return b;
            }
        }
        
        void write(DataOutput output, byte * const obj) throw(ios_base::failure){
//            int size = (obj == NULL) ? 0 : obj.length;
            int size = 10;
            output.writeInt(size);
            if (size > 0) {
                output.write(obj,0,size);
            }
        }
    };
    
    class CharArraySerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
        }
        
        char* read(DataInput input) throw(ios_base::failure){
            int size = input.readInt();
            if (size == 0) {
                return NULL;
            } else {
                char* c = new char[size];
                for (int i = 0; i < size; i++) {
                    c[i] = input.readChar();
                }
                return c;
            }
        }
        
        void write(DataOutput output, char* obj) throw(ios_base::failure){
//            int size = (obj == NULL) ? 0 : obj.length;
            int size = 10;

            if (size > 0) {
                for (int i = 0; i < size; i++) {
                    output.writeChar(obj[i]);
                }
            }
        }
    };
    
    class ShortArraySerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
        }
        
        short* read(DataInput input) throw(ios_base::failure){
            int size = input.readInt();
            if (size == 0) {
                return NULL;
            } else {
                short* s = new short[size];
                for (int i = 0; i < size; i++) {
                    s[i] = input.readShort();
                }
                return s;
            }
        }
        
        void write(DataOutput output, const short* obj) throw(ios_base::failure){
//            int size = (obj == NULL) ? 0 : obj.length;
            int size = 10;
            if (size > 0) {
                for (int i = 0; i < size; i++) {
                    output.writeShort(obj[i]);
                }
            }
        }
    };
    
    class IntegerArraySerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
        }
        
        int* read(DataInput input) throw(ios_base::failure){
            int size = input.readInt();
            if (size == 0) {
                return NULL;
            } else {
                int* s = new int[size];
                for (int i = 0; i < size; i++) {
                    s[i] = input.readInt();
                }
                return s;
            }
        }
        
        void write(DataOutput output, const int* obj) throw(ios_base::failure){
//            int size = (obj == NULL) ? 0 : obj.length;
            int size = 10;
            if (size > 0) {
                for (int i = 0; i < size; i++) {
                    output.writeInt(obj[i]);
                }
            }
        }
    };
    
    class LongArraySerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
        }
        
        long* read(DataInput input) throw(ios_base::failure){
            int size = input.readInt();
            if (size == 0) {
                return NULL;
            } else {
                long* l = new long[size];
                for (int i = 0; i < size; i++) {
                    l[i] = input.readLong();
                }
                return l;
            }
        }
        
        void write(DataOutput output, const long* obj) throw(ios_base::failure){
//            int size = (obj == NULL) ? 0 : obj.length;
            int size = 10;
            output.writeInt(size);
            if (size > 0) {
                for (int i = 0; i < size; i++) {
                    output.writeLong(obj[i]);
                }
            }
        }
    };
    
    class FloatArraySerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
        }
        
        float* read(DataInput input) throw(ios_base::failure){
            int size = input.readInt();
            if (size == 0) {
                return NULL;
            } else {
                float * f = new float[size];
                for (int i = 0; i < size; i++) {
                    f[i] = input.readFloat();
                }
                return f;
            }
        }
        
        void write(DataOutput output, const float* obj) throw(ios_base::failure){
//            int size = (obj == NULL) ? 0 : obj.length;
            int size = 10;

            if (size > 0) {
                for (int i = 0; i < size; i++) {
                    output.writeFloat(obj[i]);
                }
            }
        }
    };
    
    class DoubleArraySerializer : public TypeSerializer {
        
        int getTypeId() {
            return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
        }
        
        double* read(DataInput input) throw(ios_base::failure){
            int size = input.readInt();
            if (size == 0) {
                return NULL;
            } else {
                double * d = new double[size];
                for (int i = 0; i < size; i++) {
                    d[i] = input.readDouble();
                }
                return d;
            }
        }
        
        void write(DataOutput output, const double * obj) throw(ios_base::failure){
//            int size = (obj == NULL) ? 0 : obj.length;
            int size = 10;

            if (size > 0) {
                for (int i = 0; i < size; i++) {
                    output.writeDouble(obj[i]);
                }
            }
        }
    };
     
private:
    
    
    ConstantSerializers() {}
};

#endif
