//
//  ConstantSerializers.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CONSTANT_SERIALIZERS
#define HAZELCAST_CONSTANT_SERIALIZERS

#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {

            typedef unsigned char byte;

            template<typename T>
            int getSerializerId(const T& t) {
                return t.getSerializerId();
            };

            int getSerializerId(byte data);

            int getSerializerId(bool data);

            int getSerializerId(char data);

            int getSerializerId(short data);

            int getSerializerId(int data);

            int getSerializerId(long data);

            int getSerializerId(float data);

            int getSerializerId(double data);

            int getSerializerId(const std::string& data);

            int getSerializerId(const std::vector<byte>&  data);

            int getSerializerId(const std::vector<char >&  data);

            int getSerializerId(const std::vector<short >&  data);

            int getSerializerId(const std::vector<int>&  data);

            int getSerializerId(const std::vector<long >&  data);

            int getSerializerId(const std::vector<float >&  data);

            int getSerializerId(const std::vector<double >&  data);

        }
    }
}

#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
