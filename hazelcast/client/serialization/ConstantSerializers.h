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
            class ClassDefinitionWriter;

            class MorphingPortableReader;

            class BufferedDataOutput;

            class BufferedDataInput;

            class PortableReader;

            class PortableWriter;

            typedef unsigned char byte;

            template<typename T>
            int getTypeId(T t) {
                return 0;
            };

            template<typename T>
            int getFactoryId(T t) {
                return 0;
            }

            template<typename T>
            int getClassId(T t) {
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

            /*****************************************************************************/
            /*****************************************************************************/
            void writePortable(ClassDefinitionWriter& dataOutput, byte data);

            void writePortable(ClassDefinitionWriter& dataOutput, bool data);

            void writePortable(ClassDefinitionWriter& dataOutput, char data);

            void writePortable(ClassDefinitionWriter& dataOutput, short data);

            void writePortable(ClassDefinitionWriter& dataOutput, int data);

            void writePortable(ClassDefinitionWriter& dataOutput, long data);

            void writePortable(ClassDefinitionWriter& dataOutput, float data);

            void writePortable(ClassDefinitionWriter& dataOutput, double data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::string&   data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::vector<byte>&  data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::vector<char >&  data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::vector<short >&  data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::vector<int>&  data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::vector<long >&  data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::vector<float >&  data);

            void writePortable(ClassDefinitionWriter& dataOutput, const std::vector<double >&  data);
            /*****************************************************************************/
            /*****************************************************************************/
            void writePortable(BufferedDataOutput& dataOutput, byte data);

            void writePortable(BufferedDataOutput& dataOutput, bool data);

            void writePortable(BufferedDataOutput& dataOutput, char data);

            void writePortable(BufferedDataOutput& dataOutput, short data);

            void writePortable(BufferedDataOutput& dataOutput, int data);

            void writePortable(BufferedDataOutput& dataOutput, long data);

            void writePortable(BufferedDataOutput& dataOutput, float data);

            void writePortable(BufferedDataOutput& dataOutput, double data);

            void writePortable(BufferedDataOutput& dataOutput, const std::string&   data);

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<byte>&  data);

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<char >&  data);

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<short >&  data);

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<int>&  data);

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<long >&  data);

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<float >&  data);

            void writePortable(BufferedDataOutput& dataOutput, const std::vector<double >&  data);
            /*****************************************************************************/
            /*****************************************************************************/
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

            /*****************************************************************************/
            void readPortable(PortableReader& portableReader, byte& data);

            void readPortable(PortableReader& portableReader, bool& data);

            void readPortable(PortableReader& portableReader, char& data);

            void readPortable(PortableReader& portableReader, short & data);

            void readPortable(PortableReader& portableReader, int& data);

            void readPortable(PortableReader& portableReader, long & data);

            void readPortable(PortableReader& portableReader, float & data);

            void readPortable(PortableReader& portableReader, double & data);

            void readPortable(PortableReader& portableReader, std::string&  data);

            void readPortable(PortableReader& portableReader, std::vector<byte>& data);

            void readPortable(PortableReader& portableReader, std::vector<char >& data);

            void readPortable(PortableReader& portableReader, std::vector<short >& data);

            void readPortable(PortableReader& portableReader, std::vector<int>& data);

            void readPortable(PortableReader& portableReader, std::vector<long >& data);

            void readPortable(PortableReader& portableReader, std::vector<float >& data);

            void readPortable(PortableReader& portableReader, std::vector<double >& data);

            /*****************************************************************************/
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


            /*****************************************************************************/

            void readPortable(BufferedDataInput& dataInput, byte& data);

            void readPortable(BufferedDataInput& dataInput, bool& data);

            void readPortable(BufferedDataInput& dataInput, char& data);

            void readPortable(BufferedDataInput& dataInput, short & data);

            void readPortable(BufferedDataInput& dataInput, int& data);

            void readPortable(BufferedDataInput& dataInput, long & data);

            void readPortable(BufferedDataInput& dataInput, float & data);

            void readPortable(BufferedDataInput& dataInput, double & data);

            void readPortable(BufferedDataInput& dataInput, std::string&  data);

            void readPortable(BufferedDataInput& dataInput, std::vector<byte>& data);

            void readPortable(BufferedDataInput& dataInput, std::vector<char >& data);

            void readPortable(BufferedDataInput& dataInput, std::vector<short >& data);

            void readPortable(BufferedDataInput& dataInput, std::vector<int>& data);

            void readPortable(BufferedDataInput& dataInput, std::vector<long >& data);

            void readPortable(BufferedDataInput& dataInput, std::vector<float >& data);

            void readPortable(BufferedDataInput& dataInput, std::vector<double >& data);
        }
    }
}
#endif /* HAZELCAST_CONSTANT_SERIALIZERS */
