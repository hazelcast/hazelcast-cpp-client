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
        
    auto_ptr<Portable> readPortable(string) throw(ios_base::failure) ;

    Array<byte> readByteArray(string) throw(ios_base::failure);
    
    Array<char> readCharArray(string) throw(ios_base::failure);
    
    Array<int> readIntArray(string) throw(ios_base::failure);
    
    Array<long> readLongArray(string) throw(ios_base::failure);
    
    Array<double> readDoubleArray(string) throw(ios_base::failure);
    
    Array<float> readFloatArray(string) throw(ios_base::failure);
    
    Array<short> readShortArray(string) throw(ios_base::failure);
    
    Array< auto_ptr<Portable> > readPortableArray(string) throw(ios_base::failure);
    
};

}}}
#endif	/* HAZELCAST_MORPHING_PORTABLE_READER */

