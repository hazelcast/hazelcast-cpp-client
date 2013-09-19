//
// Created by sancar koyunlu on 5/3/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.
//
// To change the template use AppCode | Preferences | File Templates.
//



#ifndef HAZELCAST_CLASS_DEFINITION_BUILDER
#define HAZELCAST_CLASS_DEFINITION_BUILDER

#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            class ClassDefinition;

            class ClassDefinitionBuilder {
            public:

                ClassDefinitionBuilder(int, int);

                ClassDefinitionBuilder& addIntField(const std::string&);

                ClassDefinitionBuilder& addLongField(const std::string&);

                ClassDefinitionBuilder& addBooleanField(const std::string&);

                ClassDefinitionBuilder& addByteField(const std::string&);

                ClassDefinitionBuilder& addCharField(const std::string&);

                ClassDefinitionBuilder& addDoubleField(const std::string&);

                ClassDefinitionBuilder& addFloatField(const std::string&);

                ClassDefinitionBuilder& addShortField(const std::string&);

                ClassDefinitionBuilder& addUTFField(const std::string&);

                ClassDefinitionBuilder& addPortableField(const std::string&, int, int);

                ClassDefinitionBuilder& addIntArrayField(const std::string&);

                ClassDefinitionBuilder& addLongArrayField(const std::string&);

                ClassDefinitionBuilder& addByteArrayField(const std::string&);

                ClassDefinitionBuilder& addCharArrayField(const std::string&);

                ClassDefinitionBuilder& addDoubleArrayField(const std::string&);

                ClassDefinitionBuilder& addFloatArrayField(const std::string&);

                ClassDefinitionBuilder& addShortArrayField(const std::string&);

                ClassDefinitionBuilder& addPortableArrayField(const std::string&, int, int);

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
