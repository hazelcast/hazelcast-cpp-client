//
//  ClassDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "ObjectDataInput.h"
#include "ObjectDataOutput.h"
#include "IOException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ClassDefinition::ClassDefinition()
            : factoryId(0), classId(0), version(-1)
            , binary(new std::vector<byte>) {
            };

            ClassDefinition::ClassDefinition(int factoryId, int classId, int version)
            : factoryId(factoryId), classId(classId), version(version)
            , binary(new std::vector<byte>) {
            };

            ClassDefinition::~ClassDefinition() {
            };

            ClassDefinition::ClassDefinition(const ClassDefinition& rhs)
            : factoryId(rhs.factoryId)
            , classId(rhs.classId)
            , version(rhs.version)
            , fieldDefinitions(rhs.fieldDefinitions)
            , fieldDefinitionsMap(rhs.fieldDefinitionsMap)
            , nestedClassDefinitions(rhs.nestedClassDefinitions) {
            };

            ClassDefinition& ClassDefinition::operator = (const ClassDefinition& rhs) {
                factoryId = rhs.factoryId;
                classId = rhs.classId;
                version = rhs.version;
                fieldDefinitions = rhs.fieldDefinitions;
                fieldDefinitionsMap = rhs.fieldDefinitionsMap;
                nestedClassDefinitions = rhs.nestedClassDefinitions;
                return (*this);
            };

            void ClassDefinition::add(FieldDefinition& fd) {
                fieldDefinitions.push_back(fd);
                fieldDefinitionsMap[fd.getName().c_str()] = fd;
            };

            void ClassDefinition::add(util::AtomicPointer<ClassDefinition> cd) {
                nestedClassDefinitions.push_back(cd);
            };

            bool ClassDefinition::isFieldDefinitionExists(const char *name) {
                return (fieldDefinitionsMap.count(name) > 0);
            }

            const FieldDefinition& ClassDefinition::get(const char *name) {
                return fieldDefinitionsMap[name];
            };

            const FieldDefinition& ClassDefinition::get(int fieldIndex) {
                return fieldDefinitions[fieldIndex];
            };

            vector<util::AtomicPointer<ClassDefinition> >& ClassDefinition::getNestedClassDefinitions() {
                return nestedClassDefinitions;
            };

            bool ClassDefinition::hasField(const char *fieldName) const {
                return fieldDefinitionsMap.count(fieldName) != 0;
            };

            std::vector<std::string>  ClassDefinition::getFieldNames() const {
                std::vector<std::string> fieldNames;
                for (std::map<const char *, FieldDefinition>::const_iterator it = fieldDefinitionsMap.begin(); it != fieldDefinitionsMap.end(); ++it) {
                    fieldNames.push_back(it->first);
                }
                return fieldNames;
            }

            FieldType ClassDefinition::getFieldType(const char *fieldName) const {
                if (hasField(fieldName)) {
                    return fieldDefinitionsMap.at(fieldName).getType();
                } else {
                    throw exception::IOException("ClassDefinition::getFieldType", "field does not exist");
                }
            }

            int ClassDefinition::getFieldCount() const {
                return (int) fieldDefinitions.size();
            };


            int ClassDefinition::getFactoryId() const {
                return factoryId;
            }

            int ClassDefinition::getClassId() const {
                return classId;
            };

            int ClassDefinition::getVersion() const {
                return version;
            };


            void ClassDefinition::setVersion(int version) {
                this->version = version;
            };

            const std::vector<byte>& ClassDefinition::getBinary() const {
                return *(binary.get());
            };

            void ClassDefinition::setBinary(std::auto_ptr < std::vector<byte> > binary) {
                this->binary.reset(binary.release());
            };

            void ClassDefinition::writeData(ObjectDataOutput& dataOutput) {
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
                dataOutput.writeInt(version);
                dataOutput.writeInt(fieldDefinitions.size());
                for (int i = 0; i < fieldDefinitions.size(); i++) {
                    fieldDefinitions[i].writeData(dataOutput);
                }

                dataOutput .writeInt(int(nestedClassDefinitions.size()));
                for (int i = 0; i < nestedClassDefinitions.size(); i++) {
                    nestedClassDefinitions[i]->writeData(dataOutput);
                }
            };

            void ClassDefinition::readData(ObjectDataInput& dataInput) {
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
                version = dataInput.readInt();
                int size = dataInput.readInt();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(dataInput);
                    add(fieldDefinition);
                }
                size = dataInput.readInt();
                for (int i = 0; i < size; i++) {
                    util::AtomicPointer<ClassDefinition> classDefinition(new ClassDefinition);
                    classDefinition->readData(dataInput);
                    add(classDefinition);
                }
            };

        }
    }
}