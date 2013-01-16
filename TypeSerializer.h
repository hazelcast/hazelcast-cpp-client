//
//  TypeSerializer.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef Server_TypeSerializer_h
#define Server_TypeSerializer_h

class DataInput;
class DataOutput;

class TypeSerializer{
public:
    template<typename T>
    inline void write(DataOutput*, T ) throw(std::ios_base::failure){
        
    };
    
    template<typename T>
    inline T read(DataInput*) throw(std::ios_base::failure){
        
    };
    
};
/*
template<>
inline void TypeSerializer::write(DataOutput* dataOutput, byte obj ) throw(std::ios_base::failure){
    dataOutput->writeByte(obj);
};

template<>
inline byte TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readByte();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, bool obj ) throw(std::ios_base::failure){
    dataOutput->writeBoolean(obj);
};

template<>
inline bool TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readBoolean();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, char obj ) throw(std::ios_base::failure){
    dataOutput->writeChar(obj);
};

template<>
inline char TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readChar();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, short obj ) throw(std::ios_base::failure){
    dataOutput->writeShort(obj);
};

template<>
inline short TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readShort();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, int obj ) throw(std::ios_base::failure){
    dataOutput->writeInt(obj);
};

template<>
inline int TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readInt();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, long obj ) throw(std::ios_base::failure){
    dataOutput->writeLong(obj);
};

template<>
inline long TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readLong();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, float obj) throw(std::ios_base::failure){
    dataOutput->writeFloat(obj);
    
};

template<>
inline float TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readFloat();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, double obj ) throw(std::ios_base::failure){
    dataOutput->writeDouble(obj);
};

template<>
inline double TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readDouble();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, string obj ) throw(std::ios_base::failure){
    dataOutput->writeUTF(obj);
};

template<>
inline string TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    return dataInput->readUTF();
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, ByteArray& obj ) throw(std::ios_base::failure){
    int size = obj.length();
    dataOutput->writeInt(size);
    if (size > 0) {
        for(int i = 0; i < size ; i++){
            dataOutput->writeByte(obj[i]);
        }
    }
};

template<>
inline byte* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    int size = dataInput->readInt();
    if (size == 0) {
        return NULL;
    } else {
        byte* b = new byte[size];
        dataInput->readFully(b,0,size);
        return b;
    }
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, CharArray& obj ) throw(std::ios_base::failure){
    int size = obj.length();
    dataOutput->writeInt(size);
    if (size > 0) {
        for(int i = 0; i < size ; i++){
            dataOutput->writeChar(obj[i]);
        }
    }
};

template<>
inline char* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    int size = dataInput->readInt();
    if (size == 0) {
        return NULL;
    } else {
        char* c = new char[size];
        for (int i = 0; i < size; i++) {
            c[i] = dataInput->readChar();
        }
        return c;
    }
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, ShortArray& obj ) throw(std::ios_base::failure){
    int size = obj.length();
    dataOutput->writeInt(size);
    if (size > 0) {
        for(int i = 0; i < size ; i++){
            dataOutput->writeShort(obj[i]);
        }
    }
};

template<>
inline short* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    int size = dataInput->readInt();
    if (size == 0) {
        return NULL;
    } else {
        short* s = new short[size];
        for (int i = 0; i < size; i++) {
            s[i] = dataInput->readShort();
        }
        return s;
    }
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, IntegerArray& obj ) throw(std::ios_base::failure){
    int size = obj.length();
    dataOutput->writeInt(size);
    if (size > 0) {
        for(int i = 0; i < size ; i++){
            dataOutput->writeInt(obj[i]);
        }
    }
};

template<>
inline int* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    int size = dataInput->readInt();
    if (size == 0) {
        return NULL;
    } else {
        int* s = new int[size];
        for (int i = 0; i < size; i++) {
            s[i] = dataInput->readInt();
        }
        return s;
    }
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, LongArray& obj ) throw(std::ios_base::failure){
    int size = obj.length();
    dataOutput->writeInt(size);
    if (size > 0) {
        for(int i = 0; i < size ; i++){
            dataOutput->writeLong(obj[i]);
        }
    }
};

template<>
inline long* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    int size = dataInput->readInt();
    if (size == 0) {
        return NULL;
    } else {
        long* l = new long[size];
        for (int i = 0; i < size; i++) {
            l[i] = dataInput->readLong();
        }
        return l;
    }
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, FloatArray& obj ) throw(std::ios_base::failure){
    int size = obj.length();
    dataOutput->writeInt(size);
    if (size > 0) {
        for(int i = 0; i < size ; i++){
            dataOutput->writeFloat(obj[i]);
        }
    }
};

template<>
inline float* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    int size = dataInput->readInt();
    if (size == 0) {
        return NULL;
    } else {
        float * f = new float[size];
        for (int i = 0; i < size; i++) {
            f[i] = dataInput->readFloat();
        }
        return f;
    }
};

template<>
inline void TypeSerializer::write(DataOutput* dataOutput, DoubleArray& obj ) throw(std::ios_base::failure){
    int size = obj.length();
    dataOutput->writeInt(size);
    if (size > 0) {
        for(int i = 0; i < size ; i++){
            dataOutput->writeDouble(obj[i]);
        }
    }
};

template<>
inline double* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    int size = dataInput->readInt();
    if (size == 0) {
        return NULL;
    } else {
        double * d = new double[size];
        for (int i = 0; i < size; i++) {
            d[i] = dataInput->readDouble();
        }
        return d;
    }
};
*/

/* May implement for portable
template<>
inline void TypeSerializer::write(DataOutput* dataOutput, Portable& obj ) throw(std::ios_base::failure){
    //TODO
};

template<>
inline double* TypeSerializer::read(DataInput* dataInput) throw(std::ios_base::failure){
    //TODO
};
*/
#endif
