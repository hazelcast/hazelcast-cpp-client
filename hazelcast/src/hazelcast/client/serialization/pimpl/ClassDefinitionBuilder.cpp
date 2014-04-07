//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//


#include "hazelcast/client/serialization/pimpl/ClassDefinitionBuilder.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/IOUtil.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId)
                : cd(new ClassDefinition(factoryId, classId, -1))
                , index(0)
                , done(false) {
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addIntField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_INT);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addLongField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_LONG);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addBooleanField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_BOOLEAN);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addByteField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_BYTE);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addCharField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_CHAR);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addDoubleField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_DOUBLE);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addFloatField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_FLOAT);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addShortField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_SHORT);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addUTFField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_UTF);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableField(const std::string &fieldName, int factoryId, int classId) {
                    check();
                    if (classId == Data::NO_CLASS_ID) {
                        throw  exception::IOException("ClassDefinitionBuilder::addPortableField", "Portable class id cannot be zero!");
                    }
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_PORTABLE, factoryId, classId);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addIntArrayField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_INT_ARRAY);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addLongArrayField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_LONG_ARRAY);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addByteArrayField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_BYTE_ARRAY);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addCharArrayField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_CHAR_ARRAY);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addDoubleArrayField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_DOUBLE_ARRAY);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addFloatArrayField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_FLOAT_ARRAY);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addShortArrayField(const std::string &fieldName) {
                    check();
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_SHORT_ARRAY);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinitionBuilder &ClassDefinitionBuilder::addPortableArrayField(const std::string &fieldName, int factoryId, int classId) {
                    check();
                    if (classId == Data::NO_CLASS_ID) {
                        throw  exception::IOException("ClassDefinitionBuilder::addPortableArrayField", "Portable class id cannot be zero!");
                    }
                    FieldDefinition fd(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY, factoryId, classId);
                    cd->add(fd);
                    return *this;
                }

                ClassDefinition *ClassDefinitionBuilder::build() {
                    done = true;
                    return cd;
                }

                void ClassDefinitionBuilder::check() {
                    if (done) {
                        std::string message = "ClassDefinition is already built for " + util::IOUtil::to_string(cd->getClassId());
                        throw exception::IOException("ClassDefinitionBuilder::check", message);
                    }

                }
            }
        }
    }
}
