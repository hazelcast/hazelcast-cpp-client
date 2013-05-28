#include "ConstantSerializers.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            int getTypeId(byte data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getTypeId(bool data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getTypeId(char data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            };

            int getTypeId(short data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT;
            };

            int getTypeId(int data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER;
            };

            int getTypeId(long data) {
                return SerializationConstants::CONSTANT_TYPE_LONG;
            };

            int getTypeId(float data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT;
            };

            int getTypeId(double data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE;
            };

            int getTypeId(const std::string&   data) {
                return SerializationConstants::CONSTANT_TYPE_STRING;
            };

            int getTypeId(const std::vector<byte>&  data) {
                return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            };

            int getTypeId(const std::vector<char >&  data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
            };

            int getTypeId(const std::vector<short >&  data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
            };

            int getTypeId(const std::vector<int>&  data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
            };

            int getTypeId(const std::vector<long >&  data) {
                return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
            };

            int getTypeId(const std::vector<float >&  data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
            };

            int getTypeId(const std::vector<double >&  data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
            };

        }
    }
}