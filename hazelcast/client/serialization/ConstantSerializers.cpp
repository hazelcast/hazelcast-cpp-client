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
            };

            int getClassId(bool data) {
            };

            int getClassId(char data) {
            };

            int getClassId(short data) {
            };

            int getClassId(int data) {
            };

            int getClassId(long data) {
            };

            int getClassId(float data) {
            };

            int getClassId(double data) {
            };

            int getClassId(const std::string& data) {
            };

            int getClassId(const std::vector<byte>&  data) {
            };

            int getClassId(const std::vector<char >&  data) {
            };

            int getClassId(const std::vector<short >&  data) {
            };

            int getClassId(const std::vector<int>&  data) {
            };

            int getClassId(const std::vector<long >&  data) {
            };

            int getClassId(const std::vector<float >&  data) {
            };

            int getClassId(const std::vector<double >&  data) {
            };

            int getFactoryId(byte data) {
            };

            int getFactoryId(bool data) {
            };

            int getFactoryId(char data) {
            };

            int getFactoryId(short data) {
            };

            int getFactoryId(int data) {
            };

            int getFactoryId(long data) {
            };

            int getFactoryId(float data) {
            };

            int getFactoryId(double data) {
            };

            int getFactoryId(const std::string& data) {
            };

            int getFactoryId(const std::vector<byte>&  data) {
            };

            int getFactoryId(const std::vector<char >&  data) {
            };

            int getFactoryId(const std::vector<short >&  data) {
            };

            int getFactoryId(const std::vector<int>&  data) {
            };

            int getFactoryId(const std::vector<long >&  data) {
            };

            int getFactoryId(const std::vector<float >&  data) {
            };

            int getFactoryId(const std::vector<double >&  data) {
            };


        }
    }
}