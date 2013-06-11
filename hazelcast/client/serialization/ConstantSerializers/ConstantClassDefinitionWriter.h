//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CONSTANT_CDW
#define HAZELCAST_CONSTANT_CDW

#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class Data;

            class ClassDefinitionWriter;

            class NullPortable;

            void operator <<(ClassDefinitionWriter& dataOutput, byte data);

            void operator <<(ClassDefinitionWriter& dataOutput, bool data);

            void operator <<(ClassDefinitionWriter& dataOutput, char data);

            void operator <<(ClassDefinitionWriter& dataOutput, short data);

            void operator <<(ClassDefinitionWriter& dataOutput, int data);

            void operator <<(ClassDefinitionWriter& dataOutput, long data);

            void operator <<(ClassDefinitionWriter& dataOutput, float data);

            void operator <<(ClassDefinitionWriter& dataOutput, double data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::string&   data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::vector<byte>&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::vector<char >&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::vector<short >&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::vector<int>&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::vector<long >&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::vector<float >&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const std::vector<double >&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const NullPortable&  data);

            void operator <<(ClassDefinitionWriter& dataOutput, const Data&  data);

        }
    }
}


#endif //HAZELCAST_CONSTANT_CDW
