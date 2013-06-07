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
            int getTypeSerializerId(const T& t) {
                return t.getTypeSerializerId();
            };

            template<typename T>
            int getFactoryId(const T& t) {
                return t.getFactoryId();
            }

            template<typename T>
            int getClassId(const T& t) {
                return t.getClassId();
            }

            int getTypeSerializerId(byte data);

            int getTypeSerializerId(bool data);

            int getTypeSerializerId(char data);

            int getTypeSerializerId(short data);

            int getTypeSerializerId(int data);

            int getTypeSerializerId(long data);

            int getTypeSerializerId(float data);

            int getTypeSerializerId(double data);

            int getTypeSerializerId(const std::string& data);

            int getTypeSerializerId(const std::vector<byte>&  data);

            int getTypeSerializerId(const std::vector<char >&  data);

            int getTypeSerializerId(const std::vector<short >&  data);

            int getTypeSerializerId(const std::vector<int>&  data);

            int getTypeSerializerId(const std::vector<long >&  data);

            int getTypeSerializerId(const std::vector<float >&  data);

            int getTypeSerializerId(const std::vector<double >&  data);

            int getClassId(byte data);

            int getClassId(bool data);

            int getClassId(char data);

            int getClassId(short data);

            int getClassId(int data);

            int getClassId(long data);

            int getClassId(float data);

            int getClassId(double data);

            int getClassId(const std::string& data);

            int getClassId(const std::vector<byte>&  data);

            int getClassId(const std::vector<char >&  data);

            int getClassId(const std::vector<short >&  data);

            int getClassId(const std::vector<int>&  data);

            int getClassId(const std::vector<long >&  data);

            int getClassId(const std::vector<float >&  data);

            int getClassId(const std::vector<double >&  data);

            int getFactoryId(byte data);

            int getFactoryId(bool data);

            int getFactoryId(char data);

            int getFactoryId(short data);

            int getFactoryId(int data);

            int getFactoryId(long data);

            int getFactoryId(float data);

            int getFactoryId(double data);

            int getFactoryId(const std::string& data);

            int getFactoryId(const std::vector<byte>&  data);

            int getFactoryId(const std::vector<char >&  data);

            int getFactoryId(const std::vector<short >&  data);

            int getFactoryId(const std::vector<int>&  data);

            int getFactoryId(const std::vector<long >&  data);

            int getFactoryId(const std::vector<float >&  data);

            int getFactoryId(const std::vector<double >&  data);

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
