//
//  FieldDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_FIELD_DEFINITION
#define HAZELCAST_FIELD_DEFINITION

#include "FieldType.h"
#include "SerializationConstants.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class DataInput;

            class DataOutput;


            class FieldDefinition {
            public:

                FieldDefinition();

                FieldDefinition(int, std::string, FieldType const&);

                FieldDefinition(int, std::string, FieldType const&, int, int);

                FieldType getType() const;

                std::string getName() const;

                int getIndex() const;

                int getFactoryId() const;

                int getClassId() const;

                bool operator ==(const FieldDefinition&) const;

                bool operator !=(const FieldDefinition&) const;

                void writeData(DataOutput & dataOutput);

                void readData(DataInput & dataInput);

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
