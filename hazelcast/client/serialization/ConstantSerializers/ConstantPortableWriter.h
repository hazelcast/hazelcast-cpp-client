//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CONSTANT_PW
#define HAZELCAST_CONSTANT_PW

#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class Data;

            class PortableWriter;

            class NullPortable;

            void writePortable(PortableWriter& dataOutput, byte data);

            void writePortable(PortableWriter& dataOutput, bool data);

            void writePortable(PortableWriter& dataOutput, char data);

            void writePortable(PortableWriter& dataOutput, short data);

            void writePortable(PortableWriter& dataOutput, int data);

            void writePortable(PortableWriter& dataOutput, long data);

            void writePortable(PortableWriter& dataOutput, float data);

            void writePortable(PortableWriter& dataOutput, double data);

            void writePortable(PortableWriter& dataOutput, const std::string&   data);

            void writePortable(PortableWriter& dataOutput, const std::vector<byte>&  data);

            void writePortable(PortableWriter& dataOutput, const std::vector<char >&  data);

            void writePortable(PortableWriter& dataOutput, const std::vector<short >&  data);

            void writePortable(PortableWriter& dataOutput, const std::vector<int>&  data);

            void writePortable(PortableWriter& dataOutput, const std::vector<long >&  data);

            void writePortable(PortableWriter& dataOutput, const std::vector<float >&  data);

            void writePortable(PortableWriter& dataOutput, const std::vector<double >&  data);

            void writePortable(PortableWriter& dataOutput, const NullPortable&  data);

            void writePortable(PortableWriter& dataOutput, const Data&  data);

            void operator <<(PortableWriter& dataOutput, byte data);

            void operator <<(PortableWriter& dataOutput, bool data);

            void operator <<(PortableWriter& dataOutput, char data);

            void operator <<(PortableWriter& dataOutput, short data);

            void operator <<(PortableWriter& dataOutput, int data);

            void operator <<(PortableWriter& dataOutput, long data);

            void operator <<(PortableWriter& dataOutput, float data);

            void operator <<(PortableWriter& dataOutput, double data);

            void operator <<(PortableWriter& dataOutput, const std::string&   data);

            void operator <<(PortableWriter& dataOutput, const std::vector<byte>&  data);

            void operator <<(PortableWriter& dataOutput, const std::vector<char >&  data);

            void operator <<(PortableWriter& dataOutput, const std::vector<short >&  data);

            void operator <<(PortableWriter& dataOutput, const std::vector<int>&  data);

            void operator <<(PortableWriter& dataOutput, const std::vector<long >&  data);

            void operator <<(PortableWriter& dataOutput, const std::vector<float >&  data);

            void operator <<(PortableWriter& dataOutput, const std::vector<double >&  data);

            void operator <<(PortableWriter& dataOutput, const NullPortable&  data);

            void operator <<(PortableWriter& dataOutput, const Data&  data);

        }
    }
}


#endif //HAZELCAST_CONSTANT_PW
