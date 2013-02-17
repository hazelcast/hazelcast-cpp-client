//
//  ClassDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "ClassDefinition.h"
#include "FieldDefinition.h"
#include "DataInput.h"
#include "DataOutput.h"
#include <cassert>

namespace hazelcast {
    namespace client {
        namespace serialization {

            ClassDefinition::ClassDefinition() : classId(-1), version(-1) {

            };

            ClassDefinition::ClassDefinition(int classId, int version) : classId(classId), version(version) {

            };

            ClassDefinition::ClassDefinition(const ClassDefinition& rhs) : classId(classId)
            , version(version)
            , fieldDefinitions(rhs.fieldDefinitions)
            , fieldDefinitionsMap(rhs.fieldDefinitionsMap)
            , nestedClassDefinitions(rhs.nestedClassDefinitions)
            , binary(rhs.binary) {
            };

            ClassDefinition& ClassDefinition::operator=(const ClassDefinition& rhs) {
                classId = rhs.classId;
                version = rhs.version;
                fieldDefinitions = rhs.fieldDefinitions;
                fieldDefinitionsMap = rhs.fieldDefinitionsMap;
                nestedClassDefinitions = rhs.nestedClassDefinitions;
                binary = rhs.binary;
                return (*this);
            };

            void ClassDefinition::add(FieldDefinition& fd) {
                fieldDefinitions.push_back(fd);
                fieldDefinitionsMap[fd.fieldName] = fd;
            };

            void ClassDefinition::add(boost::shared_ptr<ClassDefinition> cd) {
                nestedClassDefinitions.push_back(cd);
            };

            bool ClassDefinition::isFieldDefinitionExists(std::string name) {
                return (fieldDefinitionsMap.count(name) > 0);
            }

            const FieldDefinition& ClassDefinition::get(std::string name) {
                return fieldDefinitionsMap[name];
            };

            const FieldDefinition& ClassDefinition::get(int fieldIndex) {
                return fieldDefinitions[fieldIndex];
            };

            vector<boost::shared_ptr<ClassDefinition> >& ClassDefinition::getNestedClassDefinitions() {
                return nestedClassDefinitions;
            };

            void ClassDefinition::writeData(DataOutput & out) const {
                out.writeInt(classId);
                out.writeInt(version);
                out.writeInt((int) fieldDefinitions.size());
                for (vector<FieldDefinition>::const_iterator it = fieldDefinitions.begin(); it != fieldDefinitions.end(); it++)
                    (*it).writeData(out);
                out.writeInt((int) nestedClassDefinitions.size());
                for (vector<boost::shared_ptr<ClassDefinition> >::const_iterator it = nestedClassDefinitions.begin(); it != nestedClassDefinitions.end(); it++)
                    (*it)->writeData(out);
            };

            void ClassDefinition::readData(DataInput & in) {
                classId = in.readInt();
                version = in.readInt();
                int size = in.readInt();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(in);
                    add(fieldDefinition);
                }
                size = in.readInt();
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<ClassDefinition> classDefinition(new ClassDefinition);
                    classDefinition->readData(in);
                    add(classDefinition);
                }
            };

            int ClassDefinition::getFieldCount() {
                return (int) fieldDefinitions.size();
            };

            int ClassDefinition::getClassId() const {
                return classId;
            };

            int ClassDefinition::getVersion() const {
                return version;
            };

            std::vector<byte> ClassDefinition::getBinary() const {
                return binary;
            };

            void ClassDefinition::setBinary(std::vector<byte>& binary) {
                this->binary = binary;
            };

            bool ClassDefinition::operator ==(const ClassDefinition& rhs) const {
                if (this == &rhs) return true;
                if (classId != rhs.classId) return false;
                if (version != rhs.version) return false;
                if (fieldDefinitions != rhs.fieldDefinitions) return false;
                if (fieldDefinitionsMap != rhs.fieldDefinitionsMap) return false;
                if (nestedClassDefinitions != rhs.nestedClassDefinitions) return false;
                if (binary != rhs.binary) return false;
                return true;
            };

            bool ClassDefinition::operator !=(const ClassDefinition& rhs) const {
                return !(*this == rhs);
            };

        }
    }
}