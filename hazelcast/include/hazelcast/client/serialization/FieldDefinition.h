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

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class DataInput;

                class DataOutput;
            }

            class HAZELCAST_API FieldDefinition {
            public:

                FieldDefinition();

                FieldDefinition(int, const std::string&, FieldType const&);

                FieldDefinition(int, const std::string&, FieldType const&, int, int);

                FieldType getType() const;

                std::string getName() const;

                int getIndex() const;

                int getFactoryId() const;

                int getClassId() const;

                void writeData(pimpl::DataOutput& dataOutput);

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
#endif /* HAZELCAST_FIELD_DEFINITION */

