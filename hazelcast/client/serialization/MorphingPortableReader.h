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
    
    int readInt(string);
    
    long readLong(string);
    
    bool readBoolean(string);
    
    byte readByte(string);
    
    char readChar(string);
    
    double readDouble(string);
    
    float readFloat(string);
    
    short readShort(string);
    
    string readUTF(string); 

    std::vector<byte> readByteArray(string);
    
    std::vector<char> readCharArray(string);
    
    std::vector<int> readIntArray(string);
    
    std::vector<long> readLongArray(string);
    
    std::vector<double> readDoubleArray(string);
    
    std::vector<float> readFloatArray(string);
    
    std::vector<short> readShortArray(string);
    
    template <typename T>
    T readPortable(string fieldName) {
        if(!cd->isFieldDefinitionExists(fieldName))
           throw "throwUnknownFieldException" + fieldName;
        FieldDefinition fd = cd->get(fieldName);

        if (fd.getType() != FieldDefinition::TYPE_PORTABLE) {
            throw "IncompatibleClassChangeError";
        }
        return PortableReader::readPortable<T>(fieldName);
    };
    
    template <typename T>
    std::vector< T > readPortableArray(string fieldName) {
        if (!cd->isFieldDefinitionExists(fieldName))
            throw "throwUnknownFieldException" + fieldName;
        FieldDefinition fd = cd->get(fieldName);

        if (fd.getType() != FieldDefinition::TYPE_PORTABLE_ARRAY) {
            throw "IncompatibleClassChangeError";
        }
        return PortableReader::readPortableArray<T>(fieldName);
    };
    
};

}}}
#endif	/* HAZELCAST_MORPHING_PORTABLE_READER */

