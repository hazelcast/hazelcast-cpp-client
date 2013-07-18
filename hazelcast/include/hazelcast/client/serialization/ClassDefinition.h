//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION
#define HAZELCAST_CLASS_DEFINITION


#include "FieldDefinition.h"
#include "../../util/Util.h"
#include <iosfwd>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <cassert>

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class BufferedDataInput;

            class BufferedDataOutput;

            typedef unsigned char byte;

            class ClassDefinition {
            public:

                ClassDefinition();

                ClassDefinition(int factoryId, int classId, int version);

                ~ClassDefinition();

                void add(FieldDefinition&);

                void add(ClassDefinition *);

                bool isFieldDefinitionExists(const char *);

                const FieldDefinition& get(const char *);

                const FieldDefinition& get(int);

                vector<ClassDefinition * > &getNestedClassDefinitions();

                bool hasField(const char *fieldName) const;

                std::vector<std::string> getFieldNames() const;

                FieldType getFieldType(const char *fieldName) const;

                int getFieldCount() const;

                int getFactoryId() const;

                int getClassId() const;

                int getVersion() const;

                const std::vector<byte>& getBinary() const;

                void setBinary(std::auto_ptr < std::vector<byte> >);

                void setVersion(int);

                void writeData(BufferedDataOutput& dataOutput);

                void readData(BufferedDataInput& dataInput);

            private:
                int classId;
                int version;
                int factoryId;

                ClassDefinition(const ClassDefinition&);

                ClassDefinition& operator = (const ClassDefinition& rhs);

                vector<FieldDefinition> fieldDefinitions;
                map<const char *, FieldDefinition, util::cStrCmp> fieldDefinitionsMap;
                vector<ClassDefinition * > nestedClassDefinitions;

                std::auto_ptr< std::vector<byte> > binary;

            };

        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION */
