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


            class FieldDefinition {
                template<typename DataOutput>
                friend void writePortable(DataOutput& dataOutput, const FieldDefinition& data);

                template<typename DataInput>
                friend void readPortable(DataInput& dataInput, FieldDefinition& data);

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


            private:
                int index;
                std::string fieldName;
                FieldType type;
                int classId;
                int factoryId;
            };

            inline int getTypeSerializerId(const FieldDefinition& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            template <typename DataOutput>
            void writePortable(DataOutput & dataOutput, FieldDefinition const & data) {
                dataOutput << data.index;
                dataOutput << data.fieldName;
                dataOutput.writeByte(data.type.getId());
                dataOutput << data.factoryId;
                dataOutput << data.classId;

            };

            template <typename DataInput>
            void readPortable(DataInput & dataInput, FieldDefinition & data) {
                dataInput >> data.index;
                dataInput >> data.fieldName;
                dataInput >> data.type.id;
                dataInput >> data.factoryId;
                dataInput >> data.classId;

            };
        }
    }
}
#endif /* HAZELCAST_FIELD_DEFINITION */
