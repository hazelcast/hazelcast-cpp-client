//
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION
#define HAZELCAST_CLASS_DEFINITION


#include <iosfwd>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <cassert>
#include <boost/shared_ptr.hpp>
#include "FieldDefinition.h"
#include "ConstantSerializers.h"

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            class BufferedDataInput;

            class BufferedDataOutput;

            typedef unsigned char byte;

            class ClassDefinition {
                template<typename DataOutput>
                friend void writePortable(DataOutput& dataOutput, boost::shared_ptr<ClassDefinition> data);

                template<typename DataInput>
                friend void readPortable(DataInput& dataInput, boost::shared_ptr<ClassDefinition> data);

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

                int getFieldCount() const;

                int getFactoryId() const;

                int getClassId() const;

                int getVersion() const;

                std::vector<byte> getBinary() const;

                void setBinary(std::vector<byte>&);

                void setVersion(int);

            private:
                int classId;
                int version;
                int factoryId;

                ClassDefinition(const ClassDefinition&);

                ClassDefinition& operator = (const ClassDefinition& rhs);

                vector<FieldDefinition> fieldDefinitions;
                map<string, FieldDefinition> fieldDefinitionsMap;
                vector<boost::shared_ptr<ClassDefinition> > nestedClassDefinitions;

                std::vector<byte> binary;

            };

            template<typename DataOutput>
            void writePortable(DataOutput& dataOutput, const boost::shared_ptr<ClassDefinition> data) {
                dataOutput.writeInt(data->getFactoryId());
                dataOutput.writeInt(data->getClassId());
                dataOutput.writeInt(data->getVersion());
                dataOutput.writeInt(data->getFieldCount());
                for (vector<FieldDefinition>::const_iterator it = data->fieldDefinitions.begin(); it != data->fieldDefinitions.end(); it++)
                    dataOutput << (*it);
                dataOutput .writeInt(int(data->nestedClassDefinitions.size()));
                for (vector<boost::shared_ptr<ClassDefinition> >::const_iterator it = data->nestedClassDefinitions.begin(); it != data->nestedClassDefinitions.end(); it++)
                    dataOutput << (*it);
            };

            template<typename DataInput>
            void readPortable(DataInput& dataInput, boost::shared_ptr<ClassDefinition> data) {
                dataInput >> data->factoryId;
                dataInput >> data->classId;
                dataInput >> data->version;
                int size = 0;
                dataInput >> size;
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    dataInput >> fieldDefinition;
                    data->add(fieldDefinition);
                }
                dataInput >> size;
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<ClassDefinition> classDefinition(new ClassDefinition);
                    dataInput >> classDefinition;
                    data->add(classDefinition);
                }
            };
        }
    }
}
#endif /* HAZELCAST_CLASS_DEFINITION */
