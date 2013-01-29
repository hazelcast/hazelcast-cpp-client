//
//  Portable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE
#define HAZELCAST_PORTABLE

#include <iostream>

namespace hazelcast{ 
namespace client{
namespace serialization{

class PortableWriter;
class PortableReader;

class Portable{
public:
    virtual int getClassId() = 0; 
    
    virtual void writePortable(PortableWriter& writer) throw(std::ios_base::failure) = 0; 
    
    virtual void readPortable(PortableReader& reader) throw(std::ios_base::failure) = 0; 
    
};

}}}
#endif /* HAZELCAST_PORTABLE */
