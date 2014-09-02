//
//  ClassDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            ClassDefinition::ClassDefinition()
            : factoryId(0), classId(0), version(-1)
            , binary(new std::vector<byte>) {
            }

            ClassDefinition::ClassDefinition(int factoryId, int classId, int version)
            : factoryId(factoryId), classId(classId), version(version)
            , binary(new std::vector<byte>) {
            }

            void ClassDefinition::addFieldDef(FieldDefinition& fd) {
                fieldDefinitions.push_back(fd);
                fieldDefinitionsMap[fd.getName()] = fd;
            }

            void ClassDefinition::addClassDef(boost::shared_ptr<ClassDefinition> cd) {
                nestedClassDefinitions.push_back(cd);
            }

            const FieldDefinition& ClassDefinition::getField(const char *name) const {
                std::map<std::string, FieldDefinition>::const_iterator it;
                it = fieldDefinitionsMap.find(name);
                if (it != fieldDefinitionsMap.end()) {
                    return fieldDefinitionsMap.find(name)->second;
                }
                throw exception::IllegalArgumentException("ClassDefinition::getField", "field does not exist");
            }

            const FieldDefinition& ClassDefinition::getField(int fieldIndex) const {
                if (fieldDefinitions.size() <= (size_t)fieldIndex) {
                    return fieldDefinitions[fieldIndex];
                }
                throw exception::IllegalArgumentException("ClassDefinition::getField", "index out of bound");
            }

            std::vector<std::string> ClassDefinition::getFieldNames() const {
                std::vector<std::string> names(fieldDefinitions.size());
                std::map<std::string, FieldDefinition>::const_iterator it;
                int index = 0;
                for (it = fieldDefinitionsMap.begin(); it != fieldDefinitionsMap.end(); ++it) {
                    names[index++] = it->second.getName();
                }
                return names;
            }

            std::vector<boost::shared_ptr<ClassDefinition> >& ClassDefinition::getNestedClassDefinitions() {
                return nestedClassDefinitions;
            }

            bool ClassDefinition::hasField(const char *fieldName) const {
                return fieldDefinitionsMap.count(fieldName) != 0;
            }

            FieldType ClassDefinition::getFieldType(const char *fieldName) const {
                FieldDefinition const& fd = getField(fieldName);
                return fd.getType();
            }

            int ClassDefinition::getFieldClassId(const char *fieldName) const {
                FieldDefinition const& fd = getField(fieldName);
                return fd.getClassId();
            }

            int ClassDefinition::getFieldCount() const {
                return (int)fieldDefinitions.size();
            }


            int ClassDefinition::getFactoryId() const {
                return factoryId;
            }

            int ClassDefinition::getClassId() const {
                return classId;
            }

            int ClassDefinition::getVersion() const {
                return version;
            }

            void ClassDefinition::setVersionIfNotSet(int version) {
                if (getVersion() < 0) {
                    this->version = version;
                    std::vector<FieldDefinition>::iterator it;
                    for (it = fieldDefinitions.begin(); it != fieldDefinitions.end(); ++it) {
                        it->setVersionIfNotSet(version);
                    }
                }
            }

            int ClassDefinition::getFieldVersion(const char *fieldName) const {
                FieldDefinition const& fd = getField(fieldName);
                return fd.getVersion();
            }

            const std::vector<byte>& ClassDefinition::getBinary() const {
                return *(binary.get());
            }

            void ClassDefinition::setBinary(std::auto_ptr<std::vector<byte> > binary) {
                this->binary.reset(binary.release());
            }

            void ClassDefinition::writeData(pimpl::DataOutput& dataOutput) {
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
                dataOutput.writeInt(version);
                dataOutput.writeInt(fieldDefinitions.size());
                std::vector<FieldDefinition>::iterator it;
                for (it = fieldDefinitions.begin(); it != fieldDefinitions.end(); ++it) {
                    it->writeData(dataOutput);
                }

                dataOutput.writeInt(nestedClassDefinitions.size());
                std::vector<boost::shared_ptr<ClassDefinition> >::iterator cdIt;
                for (cdIt = nestedClassDefinitions.begin(); cdIt != nestedClassDefinitions.end(); ++cdIt) {
                    (*cdIt)->writeData(dataOutput);
                }
            }

            void ClassDefinition::readData(pimpl::DataInput& dataInput) {
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
                version = dataInput.readInt();
                int size = dataInput.readInt();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(dataInput);
                    addFieldDef(fieldDefinition);
                }
                size = dataInput.readInt();
                for (int i = 0; i < size; i++) {
                    boost::shared_ptr<ClassDefinition> classDefinition(new ClassDefinition());
                    classDefinition->readData(dataInput);
                    addClassDef(classDefinition);
                }
            }
        }
    }
}

