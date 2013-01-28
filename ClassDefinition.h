//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__ClassDefinition__
#define __Server__ClassDefinition__

#include <cassert>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include "ClassDefinition.h"
#include "DataSerializable.h"
#include "Array.h"
class FieldDefinition;
class DataInput;
class DataOutput;
typedef unsigned char byte;
using namespace std;
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
    
    void writeData(DataOutput&) const throw(std::ios_base::failure);
    void readData(DataInput&)throw(std::ios_base::failure);
    
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
#endif /* defined(__Server__ClassDefinition__) */
