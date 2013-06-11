//
// Created by sancar koyunlu on 5/28/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_CONSTANT_DI
#define HAZELCAST_CONSTANT_DI

#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            typedef unsigned char byte;

            class Data;

            class BufferedDataInput;

            void operator >>(BufferedDataInput& dataInput, byte& data);

            void operator >>(BufferedDataInput& dataInput, bool& data);

            void operator >>(BufferedDataInput& dataInput, char& data);

            void operator >>(BufferedDataInput& dataInput, short & data);

            void operator >>(BufferedDataInput& dataInput, int& data);

            void operator >>(BufferedDataInput& dataInput, long & data);

            void operator >>(BufferedDataInput& dataInput, float & data);

            void operator >>(BufferedDataInput& dataInput, double & data);

            void operator >>(BufferedDataInput& dataInput, std::string&  data);

            void operator >>(BufferedDataInput& dataInput, std::vector<byte>& data);

            void operator >>(BufferedDataInput& dataInput, std::vector<char >& data);

            void operator >>(BufferedDataInput& dataInput, std::vector<short >& data);

            void operator >>(BufferedDataInput& dataInput, std::vector<int>& data);

            void operator >>(BufferedDataInput& dataInput, std::vector<long >& data);

            void operator >>(BufferedDataInput& dataInput, std::vector<float >& data);

            void operator >>(BufferedDataInput& dataInput, std::vector<double >& data);

            void operator >>(BufferedDataInput& dataInput, Data& data);

        }
    }
}


#endif //HAZELCAST_CONSTANT_DI
