//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION
#define HAZELCAST_CLASS_DEFINITION

#include "ClassDefinition.h"
#include "DataSerializable.h"
#include "FieldDefinition.h"
#include "../Array.h"
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <cassert>

using namespace std;

namespace hazelcast{ 
namespace client{
namespace serialization{

class DataInput;
class DataOutput;

typedef unsigned char byte;

//TODO ask fieldDefinitions vector needed
class ClassDefinition : public DataSerializable{
public:
    ClassDefinition();
    ClassDefinition(const ClassDefinition&);
    
    ClassDefinition& operator=(const ClassDefinition& rhs);
    
    void add(FieldDefinition&);
    void add(ClassDefinition*);
    
    bool isFieldDefinitionExists(std::string);
    const FieldDefinition& get(std::string);
    const FieldDefinition& get(int);
    
    const vector<ClassDefinition*>& getNestedClassDefinitions();
    
    void writeData(DataOutput&) const;
    void readData(DataInput&);
    
    int getFieldCount();
    int getClassId() const;
    int getVersion() const;
    Array<byte> getBinary() const;
    
    void setBinary(Array<byte>&);
    
    bool operator==(const ClassDefinition&) const;
    bool operator!=(const ClassDefinition&) const;
    
    int classId;
    int version;
private:
    
    
    vector<FieldDefinition> fieldDefinitions;
    map<std::string, FieldDefinition> fieldDefinitionsMap;
    vector<ClassDefinition*> nestedClassDefinitions;//TODO ask if equaliy is important
    
    Array<byte> binary;
    
};

}}}
#endif /* HAZELCAST_CLASS_DEFINITION */
