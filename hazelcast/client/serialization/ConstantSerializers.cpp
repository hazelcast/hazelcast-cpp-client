#include "ConstantSerializers.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            int getTypeSerializerId(byte data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getTypeSerializerId(bool data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getTypeSerializerId(char data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            };

            int getTypeSerializerId(short data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT;
            };

            int getTypeSerializerId(int data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER;
            };

            int getTypeSerializerId(long data) {
                return SerializationConstants::CONSTANT_TYPE_LONG;
            };

            int getTypeSerializerId(float data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT;
            };

            int getTypeSerializerId(double data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE;
            };

            int getTypeSerializerId(const std::string&   data) {
                return SerializationConstants::CONSTANT_TYPE_STRING;
            };

            int getTypeSerializerId(const std::vector<byte>&  data) {
                return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            };

            int getTypeSerializerId(const std::vector<char >&  data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
            };

            int getTypeSerializerId(const std::vector<short >&  data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
            };

            int getTypeSerializerId(const std::vector<int>&  data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
            };

            int getTypeSerializerId(const std::vector<long >&  data) {
                return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
            };

            int getTypeSerializerId(const std::vector<float >&  data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
            };

            int getTypeSerializerId(const std::vector<double >&  data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
            };

            int getClassId(byte data) {
                return -1;
            };

            int getClassId(bool data) {
                return -1;
            };

            int getClassId(char data) {
                return -1;
            };

            int getClassId(short data) {
                return -1;
            };

            int getClassId(int data) {
                return -1;
            };

            int getClassId(long data) {
                return -1;
            };

            int getClassId(float data) {
                return -1;
            };

            int getClassId(double data) {
                return -1;
            };

            int getClassId(const std::string& data) {
                return -1;
            };

            int getClassId(const std::vector<byte>&  data) {
                return -1;
            };

            int getClassId(const std::vector<char >&  data) {
                return -1;
            };

            int getClassId(const std::vector<short >&  data) {
                return -1;
            };

            int getClassId(const std::vector<int>&  data) {
                return -1;
            };

            int getClassId(const std::vector<long >&  data) {
                return -1;
            };

            int getClassId(const std::vector<float >&  data) {
                return -1;
            };

            int getClassId(const std::vector<double >&  data) {
                return -1;
            };

            int getFactoryId(byte data) {
                return -1;
            };

            int getFactoryId(bool data) {
                return -1;
            };

            int getFactoryId(char data) {
                return -1;
            };

            int getFactoryId(short data) {
                return -1;
            };

            int getFactoryId(int data) {
                return -1;
            };

            int getFactoryId(long data) {
                return -1;
            };

            int getFactoryId(float data) {
                return -1;
            };

            int getFactoryId(double data) {
                return -1;
            };

            int getFactoryId(const std::string& data) {
                return -1;
            };

            int getFactoryId(const std::vector<byte>&  data) {
                return -1;
            };

            int getFactoryId(const std::vector<char >&  data) {
                return -1;
            };

            int getFactoryId(const std::vector<short >&  data) {
                return -1;
            };

            int getFactoryId(const std::vector<int>&  data) {
                return -1;
            };

            int getFactoryId(const std::vector<long >&  data) {
                return -1;
            };

            int getFactoryId(const std::vector<float >&  data) {
                return -1;
            };

            int getFactoryId(const std::vector<double >&  data) {
                return -1;
            };


        }
    }
}