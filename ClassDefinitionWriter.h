//
//  ClassDefinitionWriter.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__ClassDefinitionWriter__
#define __Server__ClassDefinitionWriter__

#include <iostream>
#include <string>
#include "PortableWriter.h"
#include "FieldDefinitionImpl.h"
#include "ClassDefinitionImpl.h"
#include "Portable.h"

using namespace std;

class ClassDefinitionWriter : public PortableWriter{
public:
    ClassDefinitionWriter(int classId,int version);
    
    void writeInt(string fieldName, int value) throw(ios_base::failure);
    
    void writeLong(string fieldName, long value) throw(ios_base::failure);
    
    void writeUTF(string fieldName, string str) throw(ios_base::failure);
    
    void writeBoolean(string fieldName, bool value) throw(ios_base::failure) ;
    
    void writeByte(string fieldName, byte value) throw(ios_base::failure) ;
    
    void writeChar(string fieldName, int value) throw(ios_base::failure) ;
    
    void writeDouble(string fieldName, double value) throw(ios_base::failure) ;
    
    void writeFloat(string fieldName, float value) throw(ios_base::failure) ;
    
    void writeShort(string fieldName, short value) throw(ios_base::failure) ;
    
    template<typename T>
    void writePortable(string fieldName, T portable) throw(ios_base::failure){
        FieldDefinitionImpl* fd = new FieldDefinitionImpl(index++, fieldName, FieldDefinitionImpl::TYPE_PORTABLE, portable.getClassId());
        addNestedField(&portable, fd);
    };
    
    void writeByteArray(string fieldName, byte* bytes) throw(ios_base::failure) ;
    
    void writeCharArray(string fieldName, char* chars) throw(ios_base::failure) ;
    
    void writeIntArray(string fieldName, int* ints) throw(ios_base::failure) ;
    
    void writeLongArray(string fieldName, long* longs) throw(ios_base::failure) ;
    
    void writeDoubleArray(string fieldName, double* values) throw(ios_base::failure) ;
    
    void writeFloatArray(string fieldName, float* values) throw(ios_base::failure) ;
    
    void writeShortArray(string fieldName, short* values) throw(ios_base::failure) ;
    
    template<typename T>
    void writePortableArray(string fieldName, T* portables) throw(ios_base::failure){
        if (portables == NULL) {
            throw "Illegal Argument Exception";
        }
        T* p = portables;
        int classId = p->getClassId();
        //    for (int i = 1; i < portables.length; i++) {//TODO
        for (int i = 1; i < 2; i++) {
            if (portables[i].getClassId() != classId) {
                throw "Illegal Argument Exception";
            }
        }
        FieldDefinitionImpl* fd = new FieldDefinitionImpl(index++, fieldName,
                                                          FieldDefinitionImpl::TYPE_PORTABLE_ARRAY, classId);
        addNestedField(p, fd);
    };
    
    ClassDefinitionImpl cd;
private:
    void addNestedField(Portable* p, FieldDefinitionImpl* fd) throw(ios_base::failure) ;

    int index;
    
    
};

#endif /* defined(__Server__ClassDefinitionWriter__) */
