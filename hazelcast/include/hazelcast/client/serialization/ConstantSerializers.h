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
#include "BufferedDataOutput.h"
#include "BufferedDataInput.h"
#include <vector>
#include <string>
#include <ostream>

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

            template<typename T>
            inline void writeData(BufferedDataOutput& portableWriter, const T& data) {
                data.writeData(portableWriter);
            };

            template<typename T>
            inline void readData(BufferedDataInput& portableReader, T& data) {
                data.readData(portableReader);
            };

            template<typename W, typename T>
            inline void writePortable(W& portableWriter, const T& data) {
                data.writePortable(portableWriter);
            };

            template<typename R, typename T>
            inline void readPortable(R& portableReader, T& data) {
                data.readPortable(portableReader);
            };


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

        }
    }
}

#include "ConstantSerializers/ConstantClassDefinitionWriter.h"
#include "ConstantSerializers/ConstantPortableWriter.h"
#include "ConstantSerializers/ConstantsPortableReader.h"
#include "ConstantSerializers/ConstantsMorphingPortableReader.h"

#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
