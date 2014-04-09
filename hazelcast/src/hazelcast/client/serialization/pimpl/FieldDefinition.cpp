//
//  FieldDefinition.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
#include "hazelcast/client/serialization/pimpl/FieldDefinition.h"
#include "hazelcast/client/serialization/pimpl/DataInput.h"
#include "hazelcast/client/serialization/pimpl/DataOutput.h"


namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                FieldDefinition::FieldDefinition() {

                };

                FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type)
                : index(index)
                , fieldName(fieldName)
                , type(type)
                , classId(-1)
                , factoryId(0) {
                };

                FieldDefinition::FieldDefinition(int index, const std::string &fieldName, FieldType const &type, int factoryId, int classId)
                : index(index)
                , fieldName(fieldName)
                , type(type)
                , classId(classId)
                , factoryId(factoryId) {
                };

                FieldType FieldDefinition::getType() const {
                    return type;
                };

                std::string FieldDefinition::getName() const {
                    return fieldName;
                };

                int FieldDefinition::getIndex() const {
                    return index;
                };

                int FieldDefinition::getFactoryId() const {
                    return factoryId;
                }

                int FieldDefinition::getClassId() const {
                    return classId;
                };


                void FieldDefinition::writeData(DataOutput &dataOutput) {
                    dataOutput.writeInt(index);
                    dataOutput.writeUTF(fieldName);
                    dataOutput.writeByte(type.getId());
                    dataOutput.writeInt(factoryId);
                    dataOutput.writeInt(classId);
                };

                void FieldDefinition::readData(DataInput &dataInput) {
                    index = dataInput.readInt();
                    fieldName = dataInput.readUTF();
                    type.id = dataInput.readByte();
                    factoryId = dataInput.readInt();
                    classId = dataInput.readInt();

                };

            }
        }
    }
}
