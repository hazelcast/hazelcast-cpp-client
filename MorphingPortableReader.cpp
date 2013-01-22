/* 
 * File:   MorphingPortableReader.cpp
 * Author: msk
 * 
 * Created on January 22, 2013, 2:35 PM
 */
#include <string>
#include "Portable.h"
#include "DefaultPortableReader.h"
#include "ContextAwareDataInput.h"
#include "PortableSerializer.h"
#include "Array.h"
#include "MorphingPortableReader.h"

MorphingPortableReader::MorphingPortableReader(PortableSerializer* p, ContextAwareDataInput& cad, ClassDefinitionImpl* cd):DefaultPortableReader(p,cad,cd) {
}

int MorphingPortableReader::readInt(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    switch (fd.getType()) {
        case FieldDefinition::TYPE_INT:
            return DefaultPortableReader::readInt(fieldName);
        case FieldDefinition::TYPE_BYTE:
            return DefaultPortableReader::readByte(fieldName);
        case FieldDefinition::TYPE_CHAR:
            return DefaultPortableReader::readChar(fieldName);
        case FieldDefinition::TYPE_SHORT:
            return DefaultPortableReader::readShort(fieldName);
        default:
            throw "IncompatibleClassChangeError";
    }
};

long MorphingPortableReader::readLong(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    switch (fd.getType()) {
        case FieldDefinition::TYPE_LONG:
            return DefaultPortableReader::readLong(fieldName);
        case FieldDefinition::TYPE_INT:
            return DefaultPortableReader::readInt(fieldName);
        case FieldDefinition::TYPE_BYTE:
            return DefaultPortableReader::readByte(fieldName);
        case FieldDefinition::TYPE_CHAR:
            return DefaultPortableReader::readChar(fieldName);
        case FieldDefinition::TYPE_SHORT:
            return DefaultPortableReader::readShort(fieldName);
        default:
            throw "IncompatibleClassChangeError";
    }
};

bool MorphingPortableReader::readBoolean(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if(fd.getType() != FieldDefinition::TYPE_BOOLEAN)
            throw "IncompatibleClassChangeError";
    
    return DefaultPortableReader::readBoolean(fieldName);     
};

byte MorphingPortableReader::readByte(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if(fd.getType() != FieldDefinition::TYPE_BYTE)
            throw "IncompatibleClassChangeError";
    
    return DefaultPortableReader::readByte(fieldName);  
};

char MorphingPortableReader::readChar(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if(fd.getType() != FieldDefinition::TYPE_CHAR)
            throw "IncompatibleClassChangeError";
    
    return DefaultPortableReader::readChar(fieldName);
};

double MorphingPortableReader::readDouble(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    switch (fd.getType()) {
        case FieldDefinition::TYPE_FLOAT:
            return DefaultPortableReader::readFloat(fieldName);
        case FieldDefinition::TYPE_DOUBLE:
            return DefaultPortableReader::readDouble(fieldName);    
        case FieldDefinition::TYPE_LONG:
            return DefaultPortableReader::readLong(fieldName);
        case FieldDefinition::TYPE_INT:
            return DefaultPortableReader::readInt(fieldName);
        case FieldDefinition::TYPE_BYTE:
            return DefaultPortableReader::readByte(fieldName);
        case FieldDefinition::TYPE_CHAR:
            return DefaultPortableReader::readChar(fieldName);
        case FieldDefinition::TYPE_SHORT:
            return DefaultPortableReader::readShort(fieldName);
        default:
            throw "IncompatibleClassChangeError";
    }
};

float MorphingPortableReader::readFloat(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    switch (fd.getType()) {
        case FieldDefinition::TYPE_FLOAT:
            return DefaultPortableReader::readFloat(fieldName);
        case FieldDefinition::TYPE_INT:
            return DefaultPortableReader::readInt(fieldName);
        case FieldDefinition::TYPE_BYTE:
            return DefaultPortableReader::readByte(fieldName);
        case FieldDefinition::TYPE_CHAR:
            return DefaultPortableReader::readChar(fieldName);
        case FieldDefinition::TYPE_SHORT:
            return DefaultPortableReader::readShort(fieldName);
        default:
            throw "IncompatibleClassChangeError";
    }
};


short MorphingPortableReader::readShort(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return 0;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    switch (fd.getType()) {
        case FieldDefinition::TYPE_BYTE:
            return DefaultPortableReader::readByte(fieldName);
        case FieldDefinition::TYPE_SHORT:
            return DefaultPortableReader::readShort(fieldName);
        default:
            throw "IncompatibleClassChangeError";
    }
};

string MorphingPortableReader::readUTF(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_UTF) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readUTF(fieldName);
};

auto_ptr<Portable> MorphingPortableReader::readPortable(string fieldName) throw(ios_base::failure) {
    if(!cd->isFieldDefinitionExists(fieldName))
        return auto_ptr<Portable>();
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_PORTABLE) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readPortable(fieldName);
};

Array<byte> MorphingPortableReader::readByteArray(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_BYTE_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readByteArray(fieldName);
};

Array<char> MorphingPortableReader::readCharArray(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_CHAR_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readCharArray(fieldName);
};

Array<int> MorphingPortableReader::readIntArray(string fieldName) throw(ios_base::failure){
   if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_INT_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readIntArray(fieldName);
};

Array<long> MorphingPortableReader::readLongArray(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_LONG_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readLongArray(fieldName);
};

Array<double> MorphingPortableReader::readDoubleArray(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_DOUBLE_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readDoubleArray(fieldName);
};

Array<float> MorphingPortableReader::readFloatArray(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_FLOAT_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readFloatArray(fieldName);
};

Array<short> MorphingPortableReader::readShortArray(string fieldName) throw(ios_base::failure){
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_SHORT_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readShortArray(fieldName);
};

Array< auto_ptr<Portable> > MorphingPortableReader::readPortableArray(string fieldName) throw(ios_base::failure){//TODO
    if(!cd->isFieldDefinitionExists(fieldName))
        return NULL;
    FieldDefinitionImpl fd = cd->get(fieldName);
    
    if (fd.getType() != FieldDefinition::TYPE_PORTABLE_ARRAY) {
        throw "IncompatibleClassChangeError";
    }
    return DefaultPortableReader::readPortableArray(fieldName);
};



