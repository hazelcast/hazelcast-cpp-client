//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION
#define HAZELCAST_CLASS_DEFINITION

#include "DataSerializable.h"
#include "FieldDefinition.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <cassert>
#include <boost/shared_ptr.hpp>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataInput;

            class DataOutput;

            typedef unsigned char byte;

            class ClassDefinition : public DataSerializable {
            public:

                ClassDefinition();

                ClassDefinition(int factoryId, int classId, int version);

                void add(FieldDefinition&);

                void add(boost::shared_ptr<ClassDefinition>);

                bool isFieldDefinitionExists(std::string);

                const FieldDefinition& get(std::string);

                const FieldDefinition& get(int);

                vector<boost::shared_ptr<ClassDefinition > > &getNestedClassDefinitions();

                bool hasField(std::string& fieldName) const;

                std::vector<std::string> getFieldNames() const;

                FieldType getFieldType(std::string fieldName) const;

                int getFieldClassId(std::string fieldName) const;

                void writeData(DataOutput&) const;

                void readData(DataInput&);

                int getFieldCount() const;

                int getFactoryId() const;

                int getClassId() const;

                int getVersion() const;

                std::vector<byte> getBinary() const;

                void setBinary(std::vector<byte>&);


            private:
                int classId;
                int version;
                int factoryId;

                ClassDefinition(const ClassDefinition&);

                ClassDefinition& operator = (const ClassDefinition& rhs);

                vector<FieldDefinition> fieldDefinitions;
                map<string, FieldDefinition> fieldDefinitionsMap;
                vector<boost::shared_ptr<ClassDefinition> > nestedClassDefinitions; //TODO ask if equaliy is important

                std::vector<byte> binary;

            };

        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION */
