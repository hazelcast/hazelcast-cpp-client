//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "ClassDefinitionBuilder.h"
#include "HazelcastException.h"
#include "StringUtil.h"
#include "Data.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId) : done(false), index(0) {
                cd.reset(new ClassDefinition(factoryId, classId, -1));
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addIntField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_INT);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addLongField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_LONG);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addBooleanField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_BOOLEAN);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addByteField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_BYTE);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addCharField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_CHAR);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addDoubleField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_DOUBLE);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addFloatField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_FLOAT);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addShortField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_SHORT);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addUTFField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_UTF);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addPortableField(string fieldName, int factoryId, int classId) {
                check();
                if (classId == Data::NO_CLASS_ID) {
                    throw  HazelcastException("Portable class id cannot be zero!");
                }
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, factoryId, classId);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addIntArrayField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_INT_ARRAY);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addLongArrayField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_LONG_ARRAY);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addByteArrayField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addCharArrayField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addDoubleArrayField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addFloatArrayField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addShortArrayField(string fieldName) {
                check();
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                cd->add(fd);
                return *this;
            }

            ClassDefinitionBuilder & ClassDefinitionBuilder::addPortableArrayField(string fieldName, int factoryId, int classId) {
                check();
                if (classId == Data::NO_CLASS_ID) {
                    throw  HazelcastException("Portable class id cannot be zero!");
                }
                FieldDefinition fd = FieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY, factoryId, classId);
                cd->add(fd);
                return *this;
            }

            boost::shared_ptr<ClassDefinition>  ClassDefinitionBuilder::build() {
                done = true;
                return cd;
            }

            void ClassDefinitionBuilder::check() {
                if (done) {
                    throw hazelcast::client::HazelcastException("ClassDefinition is already built for " + hazelcast::client::util::StringUtil::to_string(cd->getClassId()));
                }

            }
        }
    }
}
