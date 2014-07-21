//
// Created by sancar koyunlu on 21/07/14.
//


#ifndef HAZELCAST_ClassDefinitionBuilder
#define HAZELCAST_ClassDefinitionBuilder


#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/serialization/FieldDefinition.h"
#include "hazelcast/client/serialization/ClassDefinition.h"
#include <vector>
#include <set>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class HAZELCAST_API ClassDefinitionBuilder {
            public:

            private:
                int index;
                int factoryId;
                int classId;
                bool done;
                std::vector<FieldDefinition> fieldDefinitions;
                std::set<ClassDefinition> nestedClassDefinitions;
            };
        }
    }
}

#endif //HAZELCAST_ClassDefinitionBuilder
