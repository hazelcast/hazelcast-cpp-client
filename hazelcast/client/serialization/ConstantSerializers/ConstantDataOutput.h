//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CONSTANT_DO
#define HAZELCAST_CONSTANT_DO

#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class Data;

            class BufferedDataOutput;

            class NullPortable;

            void operator <<(BufferedDataOutput& dataOutput, byte data);

            void operator <<(BufferedDataOutput& dataOutput, bool data);

            void operator <<(BufferedDataOutput& dataOutput, char data);

            void operator <<(BufferedDataOutput& dataOutput, short data);

            void operator <<(BufferedDataOutput& dataOutput, int data);

            void operator <<(BufferedDataOutput& dataOutput, long data);

            void operator <<(BufferedDataOutput& dataOutput, float data);

            void operator <<(BufferedDataOutput& dataOutput, double data);

            void operator <<(BufferedDataOutput& dataOutput, const std::string&   data);

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<byte>&  data);

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<char >&  data);

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<short >&  data);

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<int>&  data);

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<long >&  data);

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<float >&  data);

            void operator <<(BufferedDataOutput& dataOutput, const std::vector<double >&  data);

            void operator <<(BufferedDataOutput& dataOutput, const NullPortable&  data);

            void operator <<(BufferedDataOutput& dataOutput, const Data& data);

        }
    }
}


#endif //__ConstantDataOutput_H_
