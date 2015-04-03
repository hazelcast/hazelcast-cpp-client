//
//  ClassDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include <stdio.h>
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

            const FieldDefinition& ClassDefinition::getField(const char *name) const {
                std::map<std::string, FieldDefinition>::const_iterator it;
                it = fieldDefinitionsMap.find(name);
                if (it != fieldDefinitionsMap.end()) {
                    return fieldDefinitionsMap.find(name)->second;
                }
                char msg[200];
                sprintf(msg, "Field (%s) does not exist", 0 != name ? name : "");
                throw exception::IllegalArgumentException("ClassDefinition::getField", msg);
            }

            const FieldDefinition* ClassDefinition::getFieldIfExist(const char *fieldName) const {
                const FieldDefinition *result = 0;
                std::map<std::string, FieldDefinition>::const_iterator it;
                it = fieldDefinitionsMap.find(fieldName);
                if (it != fieldDefinitionsMap.end()) {
                    result = &fieldDefinitionsMap.find(fieldName)->second;
                }
                return result;
            }

            bool ClassDefinition::hasField(const char *fieldName) const {
                return fieldDefinitionsMap.count(fieldName) != 0;
            }

            FieldType ClassDefinition::getFieldType(const char *fieldName) const {
                FieldDefinition const& fd = getField(fieldName);
                return fd.getType();
            }

            const FieldType *ClassDefinition::getFieldTypeIfExists(const char *fieldName) const {
                const FieldType *result = 0;
                const FieldDefinition *fd = getFieldIfExist(fieldName);
                if (NULL != fd) {
                    result = &(fd->getType());
                }
                return result;
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
                }
            }

            void ClassDefinition::writeData(pimpl::DataOutput& dataOutput) {
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
                dataOutput.writeInt(version);
                dataOutput.writeShort((short)fieldDefinitions.size());
                std::vector<FieldDefinition>::iterator it;
                for (it = fieldDefinitions.begin(); it != fieldDefinitions.end(); ++it) {
                    it->writeData(dataOutput);
                }
            }

            void ClassDefinition::readData(pimpl::DataInput& dataInput) {
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
                version = dataInput.readInt();
                int size = dataInput.readShort();
                for (int i = 0; i < size; i++) {
                    FieldDefinition fieldDefinition;
                    fieldDefinition.readData(dataInput);
                    addFieldDef(fieldDefinition);
                }
            }
        }
    }
}

