//
//  Portable.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_PORTABLE
#define HAZELCAST_PORTABLE

namespace hazelcast{ 
namespace client{
namespace serialization{

class PortableWriter;
class PortableReader;

class Portable{
public:
    virtual int getClassId() = 0; 
    virtual void writePortable(PortableWriter& writer)  = 0; 
    virtual void readPortable(PortableReader& reader)  = 0; 
    
};

}}}
#endif /* HAZELCAST_PORTABLE */
