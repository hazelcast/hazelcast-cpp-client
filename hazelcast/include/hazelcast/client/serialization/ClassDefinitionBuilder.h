//
// Created by sancar koyunlu on 21/07/14.
//


#ifndef HAZELCAST_ClassDefinitionBuilder
#define HAZELCAST_ClassDefinitionBuilder

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include <vector>
#include <set>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {

            /**
            * ClassDefinitionBuilder is used to build and register ClassDefinitions manually.
            *
            * @see ClassDefinition
            * @see Portable
            * @see SerializationConfig#addClassDefinition(ClassDefinition)
            */
            class HAZELCAST_API ClassDefinitionBuilder {
            public:
                ClassDefinitionBuilder(int factoryId, int classId, int version);

                ClassDefinitionBuilder(int factoryId, int classId);

                ClassDefinitionBuilder& addIntField(const std::string& fieldName);

                ClassDefinitionBuilder& addLongField(const std::string& fieldName);

                ClassDefinitionBuilder& addUTFField(const std::string& fieldName);

                ClassDefinitionBuilder& addBooleanField(const std::string& fieldName);

                ClassDefinitionBuilder& addByteField(const std::string& fieldName);

                ClassDefinitionBuilder& addCharField(const std::string& fieldName);

                ClassDefinitionBuilder& addDoubleField(const std::string& fieldName);

                ClassDefinitionBuilder& addFloatField(const std::string& fieldName);

                ClassDefinitionBuilder& addShortField(const std::string& fieldName);

                ClassDefinitionBuilder& addByteArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addCharArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addIntArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addLongArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addDoubleArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addFloatArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addShortArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addPortableField(const std::string& fieldName, boost::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder& addPortableArrayField(const std::string& fieldName, boost::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder& addField(FieldDefinition &fieldDefinition);

                boost::shared_ptr<ClassDefinition> build();

                int getFactoryId();

                int getClassId();

                int getVersion();

            private:
                int factoryId;
                int classId;
                int version;
                int index;
                bool done;
				std::vector<FieldDefinition> fieldDefinitions;

                void addField(const std::string& fieldName , FieldType const &fieldType);

                void check();
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_ClassDefinitionBuilder
