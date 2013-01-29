/* 
 * File:   MorphingPortableReader.h
 * Author: msk
 *
 * Created on January 22, 2013, 2:34 PM
 */

#ifndef HAZELCAST_MORPHING_PORTABLE_READER
#define	HAZELCAST_MORPHING_PORTABLE_READER

#include "PortableReader.h"

namespace hazelcast{ 
namespace client{
namespace serialization{

class MorphingPortableReader : public PortableReader {
public:
    MorphingPortableReader(PortableSerializer*, DataInput&, ClassDefinition*);
    
    int readInt(string) throw(ios_base::failure);
    
    long readLong(string) throw(ios_base::failure);
    
    bool readBoolean(string) throw(ios_base::failure);
    
    byte readByte(string) throw(ios_base::failure);
    
    char readChar(string) throw(ios_base::failure);
    
    double readDouble(string) throw(ios_base::failure);
    
    float readFloat(string) throw(ios_base::failure);
    
    short readShort(string) throw(ios_base::failure);
    
    string readUTF(string) throw(ios_base::failure);
        
    template<typename T>
    auto_ptr<T> readPortable(string fieldName) throw(ios_base::failure) {
        if(!cd->isFieldDefinitionExists(fieldName))
            return auto_ptr<T>();
        FieldDefinition fd = cd->get(fieldName);

        if (fd.getType() != FieldDefinition::TYPE_PORTABLE) {
            throw "IncompatibleClassChangeError";
        }
        return PortableReader::readPortable<T>(fieldName);
    };
    
    Array<byte> readByteArray(string) throw(ios_base::failure);
    
    Array<char> readCharArray(string) throw(ios_base::failure);
    
    Array<int> readIntArray(string) throw(ios_base::failure);
    
    Array<long> readLongArray(string) throw(ios_base::failure);
    
    Array<double> readDoubleArray(string) throw(ios_base::failure);
    
    Array<float> readFloatArray(string) throw(ios_base::failure);
    
    Array<short> readShortArray(string) throw(ios_base::failure);
    
    template<typename T>
    Array< auto_ptr<T> > readPortableArray(string fieldName) throw(ios_base::failure){//TODO
        if(!cd->isFieldDefinitionExists(fieldName))
            return NULL;
        FieldDefinition fd = cd->get(fieldName);

        if (fd.getType() != FieldDefinition::TYPE_PORTABLE_ARRAY) {
            throw "IncompatibleClassChangeError";
        }
        return PortableReader::readPortableArray<T>(fieldName);
    };
    
};

}}}
#endif	/* HAZELCAST_MORPHING_PORTABLE_READER */

