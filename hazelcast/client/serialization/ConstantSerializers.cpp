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

        }
    }
}