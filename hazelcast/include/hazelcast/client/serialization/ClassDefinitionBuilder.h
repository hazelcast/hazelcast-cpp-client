//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_CLASS_DEFINITION_BUILDER
#define HAZELCAST_CLASS_DEFINITION_BUILDER

#include <iostream>
#include "ClassDefinition.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ClassDefinitionBuilder {
            public:

                ClassDefinitionBuilder(int, int);

                ClassDefinitionBuilder& addIntField(std::string);

                ClassDefinitionBuilder& addLongField(std::string);

                ClassDefinitionBuilder& addBooleanField(std::string);

                ClassDefinitionBuilder& addByteField(std::string);

                ClassDefinitionBuilder& addCharField(std::string);

                ClassDefinitionBuilder& addDoubleField(std::string);

                ClassDefinitionBuilder& addFloatField(std::string);

                ClassDefinitionBuilder& addShortField(std::string);

                ClassDefinitionBuilder& addUTFField(std::string);

                ClassDefinitionBuilder& addPortableField(std::string, int, int);

                ClassDefinitionBuilder& addIntArrayField(std::string);

                ClassDefinitionBuilder& addLongArrayField(std::string);

                ClassDefinitionBuilder& addBooleanArrayField(std::string);

                ClassDefinitionBuilder& addByteArrayField(std::string);

                ClassDefinitionBuilder& addCharArrayField(std::string);

                ClassDefinitionBuilder& addDoubleArrayField(std::string);

                ClassDefinitionBuilder& addFloatArrayField(std::string);

                ClassDefinitionBuilder& addShortArrayField(std::string);

                ClassDefinitionBuilder& addUTFArrayField(std::string);

                ClassDefinitionBuilder& addPortableArrayField(std::string, int, int);

                ClassDefinition *build();

            private:
                void check();

                ClassDefinition *cd;
                int index;
                bool done;
            };
        }
    }
}

#endif //HAZELCAST_CLASS_DEFINITION_BUILDER
