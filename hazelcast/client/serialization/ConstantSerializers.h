//
//  ConstantSerializers.h
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#ifndef HAZELCAST_CONSTANT_SERIALIZERS
#define HAZELCAST_CONSTANT_SERIALIZERS

#include "SerializationConstants.h"
#include <vector>
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {


            typedef unsigned char byte;

            template<typename T>
            int getTypeId(const T& t) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            template<typename T>
            int getFactoryId(const T& t) {
                return 0;
            }

            template<typename T>
            int getClassId(const T& t) {
                return 0;
            }

            int getTypeId(byte data);

            int getTypeId(bool data);

            int getTypeId(char data);

            int getTypeId(short data);

            int getTypeId(int data);

            int getTypeId(long data);

            int getTypeId(float data);

            int getTypeId(double data);

            int getTypeId(const std::string&   data);

            int getTypeId(const std::vector<byte>&  data);

            int getTypeId(const std::vector<char >&  data);

            int getTypeId(const std::vector<short >&  data);

            int getTypeId(const std::vector<int>&  data);

            int getTypeId(const std::vector<long >&  data);

            int getTypeId(const std::vector<float >&  data);

            int getTypeId(const std::vector<double >&  data);

        }
    }
}

#include "ConstantSerializers/ConstantClassDefinitionWriter.h"
#include "ConstantSerializers/ConstantDataOutput.h"
#include "ConstantSerializers/ConstantPortableWriter.h"
#include "ConstantSerializers/ConstantsPortableReader.h"
#include "ConstantSerializers/ConstantsMorphingPortableReader.h"
#include "ConstantSerializers/ConstantDataInput.h"

#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
