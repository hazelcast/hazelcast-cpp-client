//
//  Data.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "Data.h"
#include "DataInput.h"
#include "DataOutput.h"
#include "SerializationContext.h"
#include "ClassDefinition.h"
#include "../Array.h"


namespace hazelcast{ 
namespace client{
namespace serialization{

Data::Data():partitionHash(-1)
            ,buffer(0)
            ,type(-1)
            ,cd(NULL){
    
};

Data::Data(const Data& rhs){
    (*this) = rhs;    
};

Data::Data(const int type, Array<byte> buffer):partitionHash(-1),cd(NULL){
    this->type = type;
    this->buffer = buffer;       
};

Data& Data::operator=(const Data& rhs){
    type = rhs.type;
    buffer = rhs.buffer;
    cd = rhs.cd;
    partitionHash = rhs.partitionHash;
    return (*this);
};

bool Data::operator==(const Data& rhs) const{
    if(type != rhs.type) return false;
    if(cd != rhs.cd) return false; 
    if(partitionHash != rhs.partitionHash) return false;
    if(buffer != rhs.buffer) return false;
    return true;
};

bool Data::operator!=(const Data& rhs) const{
    return !((*this) == rhs);
};

int Data::bufferSize() const{
    return buffer.length();
};
/**
* Calculates the size of the binary after the Data is serialized.
*
* WARNING:
*
* Should be in sync with {@link #writeData(com.hazelcast.nio.ObjectDataOutput)}
*/
int Data::totalSize() const{
    int total = 0;
    total += 4; // type
    if (cd != NULL) {
        total += 4; // cd-classId
        total += 4; // cd-version
        total += 4; // cd-binary-length
        total += cd->getBinary().length(); // cd-binary
    } else {
        total += 4; // no-classId
    }
    total += 4; // buffer-size
    total += bufferSize(); // buffer
    total += 4; // partition-hash
    return total;  
};

int Data::getPartitionHash(){
    return partitionHash;
};

void Data::setPartitionHash(int partitionHash){
    this->partitionHash = partitionHash;
};

void Data::readData(DataInput& in) {
    type = in.readInt();
    std::cout << "type " << type << std::endl;
    int classId = in.readInt();
    std::cout << "classId " << classId << std::endl;
    if (classId != NO_CLASS_ID) {
        int version = in.readInt();
        SerializationContext* context = in.getSerializationContext();
        
        int classDefSize = in.readInt();

        if(context->isClassDefinitionExists(classId,version)){
            cd = context->lookup(classId, version);
            in.skipBytes(classDefSize);
        } else {
            Array<byte>  classDefBytes(classDefSize);
            in.readFully(classDefBytes);
            cd = context->createClassDefinition(classDefBytes);
        }
    }
    int size = in.readInt();
    std::cout << "size " << size << std::endl;
    if (size > 0) {
        Array<byte>  buffer(size);
        in.readFully(buffer);
        this->buffer = buffer;
        std::cout << "data => ";
        for(int i = 0 ; i < size ; i++){
            std::cout << (int)this->buffer[i];
        }
        std::cout << std::endl;
    }
    partitionHash = in.readInt();
    std::cout << "partitionHash  " << partitionHash << std::endl;
};

void Data::writeData(DataOutput& out) const {
    out.writeInt(type);
    if (cd != NULL) {
        out.writeInt(cd->getClassId());
        out.writeInt(cd->getVersion());
        Array<byte>  classDefBytes = cd->getBinary();
        out.writeInt(classDefBytes.length());
        out.write(classDefBytes);
    } else {
        out.writeInt(NO_CLASS_ID);
    }
    int len = bufferSize();
    out.writeInt(len);
    if (len > 0) {
        out.write(buffer);
    }
    out.writeInt(partitionHash);
};

}}}

