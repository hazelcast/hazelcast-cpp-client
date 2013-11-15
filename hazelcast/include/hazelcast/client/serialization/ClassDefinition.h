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
#include <map>
#include <vector>
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class DataInput;

            class DataOutput;

            typedef unsigned char byte;

            class ClassDefinition {
            public:

                ClassDefinition();

                ClassDefinition(int factoryId, int classId, int version);

                ~ClassDefinition();

                void add(FieldDefinition &);

                void add(ClassDefinition *);

                bool isFieldDefinitionExists(const char *);

                const FieldDefinition &get(const char *);

                const FieldDefinition &get(int);

                std::vector<ClassDefinition * > &getNestedClassDefinitions();

                bool hasField(const char *fieldName) const;

                std::vector<std::string> getFieldNames() const;

                FieldType getFieldType(const char *fieldName) const;

                int getFieldCount() const;

                int getFactoryId() const;

                int getClassId() const;

                int getVersion() const;

                const std::vector<byte> &getBinary() const;

                void setBinary(std::auto_ptr < std::vector<byte> >);

                void setVersion(int);

                void writeData(DataOutput &dataOutput);

                void readData(DataInput &dataInput);

            private:
                int classId;
                int version;
                int factoryId;

                ClassDefinition(const ClassDefinition &);

                ClassDefinition &operator = (const ClassDefinition &rhs);

                std::vector<FieldDefinition> fieldDefinitions;
                std::map<std::string, FieldDefinition> fieldDefinitionsMap;
                std::vector<ClassDefinition * > nestedClassDefinitions;

                std::auto_ptr< std::vector<byte> > binary;

            };

        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION */
