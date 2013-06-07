//
//  ClassDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <iostream>
#include "ClassDefinition.h"
#include "BufferedDataInput.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            ClassDefinition::ClassDefinition() : factoryId(0), classId(0), version(-1), binary(new std::vector<byte>) {
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
                fieldDefinitionsMap[fd.getName()] = fd;
            };

            void ClassDefinition::add(ClassDefinition *cd) {
                nestedClassDefinitions.push_back(cd);
            };

            bool ClassDefinition::isFieldDefinitionExists(const std::string& name) {
                return (fieldDefinitionsMap.count(name) > 0);
            }

            const FieldDefinition& ClassDefinition::get(const std::string& name) {
                return fieldDefinitionsMap[name];
            };

            const FieldDefinition& ClassDefinition::get(int fieldIndex) {
                return fieldDefinitions[fieldIndex];
            };

            vector<ClassDefinition * >& ClassDefinition::getNestedClassDefinitions() {
                return nestedClassDefinitions;
            };


            bool ClassDefinition::hasField(const string & fieldName) const {
                return fieldDefinitionsMap.count(fieldName) != 0;
            }

            std::vector<std::string>  ClassDefinition::getFieldNames() const {
                std::vector<std::string> fieldNames;
                for (std::map<std::string, FieldDefinition>::const_iterator it = fieldDefinitionsMap.begin(); it != fieldDefinitionsMap.end(); ++it) {
                    fieldNames.push_back(it->first);
                }
                return fieldNames;
            }

            FieldType ClassDefinition::getFieldType(const std::string& fieldName) const {
                if (hasField(fieldName)) {
                    FieldDefinition x = fieldDefinitionsMap.at(fieldName);
                    FieldType a = x.getType();
                    return a;
                } else {
                    throw hazelcast::client::HazelcastException("field does not exist");
                }
            }

            int ClassDefinition::getFieldClassId(const std::string& fieldName) const {
                if (hasField(fieldName)) {
                    return fieldDefinitionsMap.at(fieldName).getClassId();
                } else {
                    throw hazelcast::client::HazelcastException("field does not exist");
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

        }
    }
}