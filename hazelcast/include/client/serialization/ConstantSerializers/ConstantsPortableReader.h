//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CONSTANT_PR
#define HAZELCAST_CONSTANT_PR

#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class Data;

            class PortableReader;

            void operator >>(PortableReader& portableReader, byte& data);

            void operator >>(PortableReader& portableReader, bool& data);

            void operator >>(PortableReader& portableReader, char& data);

            void operator >>(PortableReader& portableReader, short & data);

            void operator >>(PortableReader& portableReader, int& data);

            void operator >>(PortableReader& portableReader, long & data);

            void operator >>(PortableReader& portableReader, float & data);

            void operator >>(PortableReader& portableReader, double & data);

            void operator >>(PortableReader& portableReader, std::string&  data);

            void operator >>(PortableReader& portableReader, std::vector<byte>& data);

            void operator >>(PortableReader& portableReader, std::vector<char >& data);

            void operator >>(PortableReader& portableReader, std::vector<short >& data);

            void operator >>(PortableReader& portableReader, std::vector<int>& data);

            void operator >>(PortableReader& portableReader, std::vector<long >& data);

            void operator >>(PortableReader& portableReader, std::vector<float >& data);

            void operator >>(PortableReader& portableReader, std::vector<double >& data);

            void operator >>(PortableReader& portableReader, Data& data);
        }
    }
}


#endif //HAZELCAST_CONSTANT_PR
