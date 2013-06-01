//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CONSTANT_MPR
#define HAZELCAST_CONSTANT_MPR

#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class Data;

            class MorphingPortableReader;

            void readPortable(MorphingPortableReader& portableReader, byte& data);

            void readPortable(MorphingPortableReader& portableReader, bool& data);

            void readPortable(MorphingPortableReader& portableReader, char& data);

            void readPortable(MorphingPortableReader& portableReader, short & data);

            void readPortable(MorphingPortableReader& portableReader, int& data);

            void readPortable(MorphingPortableReader& portableReader, long & data);

            void readPortable(MorphingPortableReader& portableReader, float & data);

            void readPortable(MorphingPortableReader& portableReader, double & data);

            void readPortable(MorphingPortableReader& portableReader, std::string&  data);

            void readPortable(MorphingPortableReader& portableReader, std::vector<byte>& data);

            void readPortable(MorphingPortableReader& portableReader, std::vector<char >& data);

            void readPortable(MorphingPortableReader& portableReader, std::vector<short >& data);

            void readPortable(MorphingPortableReader& portableReader, std::vector<int>& data);

            void readPortable(MorphingPortableReader& portableReader, std::vector<long >& data);

            void readPortable(MorphingPortableReader& portableReader, std::vector<float >& data);

            void readPortable(MorphingPortableReader& portableReader, std::vector<double >& data);

            void readPortable(MorphingPortableReader& portableReader, Data& data);

            void operator >>(MorphingPortableReader& portableReader, byte& data);

            void operator >>(MorphingPortableReader& portableReader, bool& data);

            void operator >>(MorphingPortableReader& portableReader, char& data);

            void operator >>(MorphingPortableReader& portableReader, short & data);

            void operator >>(MorphingPortableReader& portableReader, int& data);

            void operator >>(MorphingPortableReader& portableReader, long & data);

            void operator >>(MorphingPortableReader& portableReader, float & data);

            void operator >>(MorphingPortableReader& portableReader, double & data);

            void operator >>(MorphingPortableReader& portableReader, std::string&  data);

            void operator >>(MorphingPortableReader& portableReader, std::vector<byte>& data);

            void operator >>(MorphingPortableReader& portableReader, std::vector<char >& data);

            void operator >>(MorphingPortableReader& portableReader, std::vector<short >& data);

            void operator >>(MorphingPortableReader& portableReader, std::vector<int>& data);

            void operator >>(MorphingPortableReader& portableReader, std::vector<long >& data);

            void operator >>(MorphingPortableReader& portableReader, std::vector<float >& data);

            void operator >>(MorphingPortableReader& portableReader, std::vector<double >& data);

            void operator >>(MorphingPortableReader& portableReader, Data& data);
        }
    }
}


#endif //HAZELCAST_CONSTANT_MPR
