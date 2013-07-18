#include "hazelcast/client/serialization/ConstantSerializers.h"
#include "hazelcast/client/serialization/SerializationConstants.h"

namespace hazelcast {
    namespace client {
        namespace serialization {

            int getSerializerId(byte data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getSerializerId(bool data) {
                return SerializationConstants::CONSTANT_TYPE_BOOLEAN;
            };

            int getSerializerId(char data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR;
            };

            int getSerializerId(short data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT;
            };

            int getSerializerId(int data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER;
            };

            int getSerializerId(long data) {
                return SerializationConstants::CONSTANT_TYPE_LONG;
            };

            int getSerializerId(float data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT;
            };

            int getSerializerId(double data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE;
            };

            int getSerializerId(const std::string&   data) {
                return SerializationConstants::CONSTANT_TYPE_STRING;
            };

            int getSerializerId(const std::vector<byte>&  data) {
                return SerializationConstants::CONSTANT_TYPE_BYTE_ARRAY;
            };

            int getSerializerId(const std::vector<char >&  data) {
                return SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY;
            };

            int getSerializerId(const std::vector<short >&  data) {
                return SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY;
            };

            int getSerializerId(const std::vector<int>&  data) {
                return SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY;
            };

            int getSerializerId(const std::vector<long >&  data) {
                return SerializationConstants::CONSTANT_TYPE_LONG_ARRAY;
            };

            int getSerializerId(const std::vector<float >&  data) {
                return SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY;
            };

            int getSerializerId(const std::vector<double >&  data) {
                return SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY;
            };


        }
    }
}