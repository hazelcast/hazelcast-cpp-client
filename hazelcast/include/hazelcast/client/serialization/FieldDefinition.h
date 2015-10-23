/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
//  FieldDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_FIELD_DEFINITION
#define HAZELCAST_FIELD_DEFINITION

#include "hazelcast/client/serialization/FieldType.h"
#include <string>

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
            * FieldDefinition defines name, type, index of a field
            */
            class HAZELCAST_API FieldDefinition {
            public:

                /**
                * Constructor
                */
                FieldDefinition();

                /**
                * Constructor
                */
                FieldDefinition(int, const std::string&, FieldType const&);

                /**
                * Constructor
                */
                FieldDefinition(int index, const std::string& fieldName, FieldType const& type, int factoryId, int classId);

                /**
                * @return field type
                */
                const FieldType &getType() const;

                /**
                * @return field name
                */
                std::string getName() const;

                /**
                * @return field index
                */
                int getIndex() const;

                /**
                * @return factory id of this field's class
                */
                int getFactoryId() const;

                /**
                * @return class id of this field's class
                */
                int getClassId() const;


                /**
                * @param dataOutput writes fieldDefinition to given dataOutput
                */
                void writeData(pimpl::DataOutput& dataOutput);

                /**
                * @param dataInput reads fieldDefinition from given dataOutput
                */
                void readData(pimpl::DataInput& dataInput);

            private:
                int index;
                std::string fieldName;
                FieldType type;
                int classId;
                int factoryId;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_FIELD_DEFINITION */

