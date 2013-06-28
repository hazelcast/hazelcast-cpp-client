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

            void operator <<(PortableWriter& portableWriter, byte data);

            void operator <<(PortableWriter& portableWriter, bool data);

            void operator <<(PortableWriter& portableWriter, char data);

            void operator <<(PortableWriter& portableWriter, short data);

            void operator <<(PortableWriter& portableWriter, int data);

            void operator <<(PortableWriter& portableWriter, long data);

            void operator <<(PortableWriter& portableWriter, float data);

            void operator <<(PortableWriter& portableWriter, double data);

            void operator <<(PortableWriter& portableWriter, const std::string&   data);

            void operator <<(PortableWriter& portableWriter, const std::vector<byte>&  data);

            void operator <<(PortableWriter& portableWriter, const std::vector<char >&  data);

            void operator <<(PortableWriter& portableWriter, const std::vector<short >&  data);

            void operator <<(PortableWriter& portableWriter, const std::vector<int>&  data);

            void operator <<(PortableWriter& portableWriter, const std::vector<long >&  data);

            void operator <<(PortableWriter& portableWriter, const std::vector<float >&  data);

            void operator <<(PortableWriter& portableWriter, const std::vector<double >&  data);

            void operator <<(PortableWriter& portableWriter, const NullPortable&  data);

            void operator <<(PortableWriter& portableWriter, const Data&  data);

        }
    }
}


#endif //HAZELCAST_CONSTANT_PW
