//
//  ClassDefinitionImpl.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef __Server__ClassDefinitionImpl__
#define __Server__ClassDefinitionImpl__

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include "ClassDefinition.h"
#include "ClassDefinitionImpl.h"
#include "DataSerializable.h"
#include "FieldDefinitionImpl.h"

using namespace std;
//TODO ask fieldDefinitions vector needed
class ClassDefinitionImpl : public ClassDefinition{
public:
    ClassDefinitionImpl();
    ClassDefinitionImpl(const ClassDefinitionImpl&);
    
    ClassDefinitionImpl& operator=(const ClassDefinitionImpl& rhs);
    
    void add(FieldDefinitionImpl&);
    void add(ClassDefinitionImpl&);
    
    bool isFieldDefinitionExists(std::string);
    const FieldDefinitionImpl& get(std::string);
    const FieldDefinitionImpl& get(int);
    
    const vector<ClassDefinitionImpl>& getNestedClassDefinitions();
    
    void writeData(DataOutput&) const throw(std::ios_base::failure);
    void readData(DataInput&)throw(std::ios_base::failure);
    
    int getFieldCount();
    int getClassId();
    int getVersion();
    Array<byte> getBinary();
    
    void setBinary(Array<byte>&);
    
    bool operator==(const ClassDefinitionImpl&) const;
    bool operator!=(const ClassDefinitionImpl&) const;
    
//    friend std::ostream& operator<<(std::ostream&, const FieldDefinition&);
    int classId;
    int version;
private:
    
    
    vector<FieldDefinitionImpl> fieldDefinitions;
    map<std::string, FieldDefinitionImpl> fieldDefinitionsMap;
    vector<ClassDefinitionImpl> nestedClassDefinitions;//TODO ask if equaliy is important
    
    Array<byte> binary;
    
};
#endif /* defined(__Server__ClassDefinitionImpl__) */
