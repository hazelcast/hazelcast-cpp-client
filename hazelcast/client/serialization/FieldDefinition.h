//
//  FieldDefinition.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_FIELD_DEFINITION
#define HAZELCAST_FIELD_DEFINITION

#include "DataSerializable.h"
#include <iostream>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            class FieldDefinition : public DataSerializable {
            public:
                FieldDefinition();
                FieldDefinition(int, std::string, byte);
                FieldDefinition(int, std::string, byte, int);

                byte getType();
                std::string getName();
                int getIndex();
                int getClassId();

                void writeData(DataOutput&) const;
                void readData(DataInput&);

                bool operator==(const FieldDefinition&) const;
                bool operator!=(const FieldDefinition&) const;

                std::string fieldName;

                static const byte TYPE_PORTABLE = 0;
                static const byte TYPE_BYTE = 1;
                static const byte TYPE_BOOLEAN = 2;
                static const byte TYPE_CHAR = 3;
                static const byte TYPE_SHORT = 4;
                static const byte TYPE_INT = 5;
                static const byte TYPE_LONG = 6;
                static const byte TYPE_FLOAT = 7;
                static const byte TYPE_DOUBLE = 8;
                static const byte TYPE_UTF = 9;
                static const byte TYPE_PORTABLE_ARRAY = 10;
                static const byte TYPE_BYTE_ARRAY = 11;
                static const byte TYPE_CHAR_ARRAY = 12;
                static const byte TYPE_SHORT_ARRAY = 13;
                static const byte TYPE_INT_ARRAY = 14;
                static const byte TYPE_LONG_ARRAY = 15;
                static const byte TYPE_FLOAT_ARRAY = 16;
                static const byte TYPE_DOUBLE_ARRAY = 17;
            private:
                int index;
                byte type;
                int classId;
            };

        }
    }
}
#endif /* HAZELCAST_FIELD_DEFINITION */
