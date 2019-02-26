/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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
//  FieldDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/FieldDefinition.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            FieldDefinition::FieldDefinition()
            : index(0)
            , classId(0)
            , factoryId(0)
            , version(-1) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string& fieldName, FieldType const& type, int version)
            : index(index)
            , fieldName(fieldName)
            , type(type)
            , classId(0)
            , factoryId(0)
            , version(version) {
            }

            FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type,
                                             int factoryId, int classId, int version)
            : index(index)
            , fieldName(fieldName)
            , type(type)
            , classId(classId)
            , factoryId(factoryId)
            , version(version) {
            }

            const FieldType &FieldDefinition::getType() const {
                return type;
            }

            std::string FieldDefinition::getName() const {
                return fieldName;
            }

            int FieldDefinition::getIndex() const {
                return index;
            }

            int FieldDefinition::getFactoryId() const {
                return factoryId;
            }

            int FieldDefinition::getClassId() const {
                return classId;
            }

            void FieldDefinition::writeData(pimpl::DataOutput& dataOutput) {
                dataOutput.writeInt(index);
                dataOutput.writeUTF(&fieldName);
                dataOutput.writeByte(type.getId());
                dataOutput.writeInt(factoryId);
                dataOutput.writeInt(classId);
            }

            void FieldDefinition::readData(pimpl::DataInput& dataInput) {
                index = dataInput.readInt();
                fieldName = *dataInput.readUTF();
                type.id = dataInput.readByte();
                factoryId = dataInput.readInt();
                classId = dataInput.readInt();
            }

            bool FieldDefinition::operator==(const FieldDefinition &rhs) const {
                return fieldName == rhs.fieldName &&
                       type == rhs.type &&
                       classId == rhs.classId &&
                       factoryId == rhs.factoryId &&
                       version == rhs.version;
            }

            bool FieldDefinition::operator!=(const FieldDefinition &rhs) const {
                return !(rhs == *this);
            }

            std::ostream &operator<<(std::ostream &os, const FieldDefinition &definition) {
                os << "FieldDefinition{" << "index: " << definition.index << " fieldName: " << definition.fieldName
                   << " type: " << definition.type << " classId: " << definition.classId << " factoryId: "
                   << definition.factoryId << " version: " << definition.version;
                return os;
            }

        }
    }
}
