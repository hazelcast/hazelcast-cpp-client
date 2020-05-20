/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

                ClassDefinitionBuilder& addBooleanArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addCharArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addIntArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addLongArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addDoubleArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addUTFArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addFloatArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addShortArrayField(const std::string& fieldName);

                ClassDefinitionBuilder& addPortableField(const std::string& fieldName, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder& addPortableArrayField(const std::string& fieldName, std::shared_ptr<ClassDefinition> def);

                ClassDefinitionBuilder& addField(FieldDefinition &fieldDefinition);

                std::shared_ptr<ClassDefinition> build();

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
