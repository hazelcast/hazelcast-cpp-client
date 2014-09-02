//
// Created by sancar koyunlu on 22/07/14.
//

#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/client/serialization/ClassDefinitionBuilder.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/exception/HazelcastSerializationException.h"

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

            ClassDefinitionBuilder::ClassDefinitionBuilder(int factoryId, int classId)
            : factoryId(factoryId)
            , classId(classId)
            , version(-1)
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
                if (def->getClassId() == pimpl::Data::NO_CLASS_ID) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField", "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE, def->getFactoryId(), def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                nestedClassDefinitions.push_back(def);
                return *this;
            }

            ClassDefinitionBuilder& ClassDefinitionBuilder::addPortableArrayField(const std::string& fieldName, boost::shared_ptr<ClassDefinition> def) {
                check();
                if (def->getClassId() == pimpl::Data::NO_CLASS_ID) {
                    throw exception::IllegalArgumentException("ClassDefinitionBuilder::addPortableField", "Portable class id cannot be zero!");
                }
                FieldDefinition fieldDefinition(index++, fieldName, FieldTypes::TYPE_PORTABLE_ARRAY, def->getFactoryId(), def->getClassId(), def->getVersion());
                fieldDefinitions.push_back(fieldDefinition);
                nestedClassDefinitions.push_back(def);
                return *this;
            }

            boost::shared_ptr<ClassDefinition> ClassDefinitionBuilder::build() {
                done = true;
                boost::shared_ptr<ClassDefinition> cd(new ClassDefinition(factoryId, classId, version));

                std::vector<FieldDefinition>::iterator fdIt;
                for (fdIt = fieldDefinitions.begin(); fdIt != fieldDefinitions.end(); fdIt++) {
                    cd->addFieldDef(*fdIt);
                }

                std::vector<boost::shared_ptr<ClassDefinition> >::iterator it;
                for (it = nestedClassDefinitions.begin(); it != nestedClassDefinitions.end(); it++) {
                    cd->addClassDef(*it);
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
                FieldDefinition fieldDefinition(index++, fieldName, fieldType);
                fieldDefinitions.push_back(fieldDefinition);
            }
        }
    }
}

