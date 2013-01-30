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
        
    auto_ptr<Portable> readPortable(string);

    Array<byte> readByteArray(string);
    
    Array<char> readCharArray(string);
    
    Array<int> readIntArray(string);
    
    Array<long> readLongArray(string);
    
    Array<double> readDoubleArray(string);
    
    Array<float> readFloatArray(string);
    
    Array<short> readShortArray(string);
    
    Array< auto_ptr<Portable> > readPortableArray(string);
    
};

}}}
#endif	/* HAZELCAST_MORPHING_PORTABLE_READER */

