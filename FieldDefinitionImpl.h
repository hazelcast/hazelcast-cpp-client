//
//  FieldDefinitionImpl.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__FieldDefinitionImpl__
#define __Server__FieldDefinitionImpl__

#include <iostream>
#include <string>
#include "FieldDefinition.h"
#include "DataSerializable.h"

class FieldDefinitionImpl : public FieldDefinition{
public:
    FieldDefinitionImpl();
    FieldDefinitionImpl(int,std::string,byte);
    FieldDefinitionImpl(int,std::string,byte,int);
    
    byte getType();
    std::string getName();
    int getIndex();
    int getClassId();
    
    void writeData(DataOutput&) const throw(std::ios_base::failure);
    void readData(DataInput&) throw(std::ios_base::failure);
    
    bool operator==(const FieldDefinitionImpl&) const;
    bool operator!=(const FieldDefinitionImpl&) const;
    
    friend std::ostream& operator<<(std::ostream&, const FieldDefinition&);
    
    //TODO hascode
private:
    int index;
    std::string fieldName;
    byte type;
    int classId = -1;
};

#endif /* defined(__Server__FieldDefinitionImpl__) */
