/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//
//  ClassDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/util/Util.h"
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
                util::snprintf(msg, 200, "Field (%s) does not exist", NULL != name ? name : "");
                throw exception::IllegalArgumentException("ClassDefinition::getField", msg);
            }

            bool ClassDefinition::hasField(const char *fieldName) const {
                return fieldDefinitionsMap.count(fieldName) != 0;
            }

            FieldType ClassDefinition::getFieldType(const char *fieldName) const {
                FieldDefinition const& fd = getField(fieldName);
                return fd.getType();
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
                dataOutput.writeShort(fieldDefinitions.size());
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

