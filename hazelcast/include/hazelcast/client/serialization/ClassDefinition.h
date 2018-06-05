/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
//  ClassDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CLASS_DEFINITION
#define HAZELCAST_CLASS_DEFINITION


#include "hazelcast/client/serialization/FieldDefinition.h"
#include <map>
#include <vector>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <ostream>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class DataInput;

                class DataOutput;
            }

            /**
            * ClassDefinition defines a class schema for Portable classes. It allows to query field names, types, class id etc.
            * It can be created manually using {@link ClassDefinitionBuilder}
            * or on demand during serialization phase.
            *
            * @see Portable
            * @see ClassDefinitionBuilder
            */
            class HAZELCAST_API ClassDefinition {
            public:

                /**
                * Constructor
                */
                ClassDefinition();

                /**
                * Constructor
                * @param factoryId factory id of class
                * @param classId id of class
                * @param version version of portable class
                */
                ClassDefinition(int factoryId, int classId, int version);

                /**
                * Internal API
                * @param fieldDefinition to be added
                */
                void addFieldDef(FieldDefinition& fieldDefinition);

                /**
                * @param fieldName field name
                * @return true if this class definition contains a field named by given name
                */
                bool hasField(char const *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return field definition by given name
                * @throws IllegalArgumentException when field not found
                */
                const FieldDefinition& getField(const char *fieldName) const;

                /**
                * @param fieldName name of the field
                * @return type of given field
                * @throws IllegalArgumentException
                */
                FieldType getFieldType(const char *fieldName) const;

                /**
                * @return total field count
                */
                int getFieldCount() const;

                /**
                * @return factory id
                */
                int getFactoryId() const;

                /**
                * @return class id
                */
                int getClassId() const;

                /**
                * @return version
                */
                int getVersion() const;

                /**
                * Internal API
                * @param version portable version
                */
                void setVersionIfNotSet(int version);

                /**
                * Internal API
                * @param dataOutput
                */
                void writeData(pimpl::DataOutput& dataOutput);

                /**
                * Internal API
                * @param dataInput
                */
                void readData(pimpl::DataInput& dataInput);

                bool operator==(const ClassDefinition &rhs) const;

                bool operator!=(const ClassDefinition &rhs) const;

                friend std::ostream &operator<<(std::ostream &os, const ClassDefinition &definition);

            private:
                int factoryId;
                int classId;
                int version;

                ClassDefinition(const ClassDefinition&);

                ClassDefinition& operator=(const ClassDefinition& rhs);

                std::map<std::string, FieldDefinition> fieldDefinitionsMap;

                std::auto_ptr<std::vector<byte> > binary;

            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 


#endif /* HAZELCAST_CLASS_DEFINITION */

