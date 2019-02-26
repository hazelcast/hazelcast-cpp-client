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
// Created by sancar koyunlu on 22/07/14.
//

#include "hazelcast/util/Util.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId, int version)
            : factoryId(factoryId)
            , classId(classId)
            , version(version)
            , index(0)
            , done(false) {

            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addIntField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_INT);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addLongField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_LONG);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addUTFField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_UTF);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addBooleanField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_BOOLEAN);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addByteField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_BYTE);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addCharField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_CHAR);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addDoubleField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addFloatField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_FLOAT);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addShortField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_SHORT);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addByteArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addBooleanArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_BOOLEAN_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addCharArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addIntArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_INT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addLongArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_LONG_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addDoubleArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addFloatArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addShortArrayField(const std::string& fieldName) {
                addField(fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addPortableField(const std::string& fieldName, boost::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                              "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, def->getFactoryId(),
                                                def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addPortableArrayField(const std::string& fieldName, boost::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == 0) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField",
                                                              "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY,
                                                def->getFactoryId(), def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addField(FieldDefinition &fieldDefinition) {
                check();
                int defIndex = fieldDefinition.getIndex();
                if (index != defIndex) {
                    char buf[100];
                    util::hz_snprintf(buf, 100, "Invalid field index. Index in definition:%d, being added at index:%d",
                                      defIndex, index);
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addField", buf);
                }
                index++;
                fieldDefinitions.push_back(fieldDefinition);
                return *this;
            }

            boost::shared_ptr<ClassDefinition> ClassDefinitionBuilder::build() {
                done = true;
                boost::shared_ptr<ClassDefinition> cd(new ClassDefinition(factoryId, classId, version));

                std::vector<FieldDefinition>::iterator fdIt;
                for (fdIt = fieldDefinitions.begin(); fdIt != fieldDefinitions.end(); fdIt++) {
                    cd->addFieldDef(*fdIt);
                }
                return cd;
            }

            void ClassDefinitionBuilder::check() {
                if (done) {
                    throw exception::HazelcastSerializationException("ClassDefinitionBuilder::check", "ClassDefinition is already built for " + util::IOUtil::to_string(classId));
                }
            }

            void ClassDefinitionBuilder::addField(const std::string& fieldName, FieldType const& fieldType) {
                check();
                FieldDefinition fieldDefinition(index++, fieldName, fieldType, version);
                fieldDefinitions.push_back(fieldDefinition);
            }

            int ClassDefinitionBuilder::getFactoryId() {
                return factoryId;
            }

            int ClassDefinitionBuilder::getClassId() {
                return classId;
            }

            int ClassDefinitionBuilder::getVersion() {
                return version;
            }
        }
    }
}

