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
                FieldDefinition(int, const std::string&, FieldType const&, int, int, int);

                /**
                * @return field type
                */
                FieldType getType() const;

                /**
                * @return field name
                */
                std::string getName() const;

                /**
                * @return field index
                */
                int getIndex() const;

                /**
                * @return version of this field's class
                */
                int getVersion() const;

                /**
                * @return factory id of this field's class
                */
                int getFactoryId() const;

                /**
                * @return class id of this field's class
                */
                int getClassId() const;

                /**
                * sets version if not set
                * @param version
                */
                void setVersionIfNotSet(int version);

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
                int version;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_FIELD_DEFINITION */

