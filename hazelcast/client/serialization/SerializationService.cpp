//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "SerializationService.h"
#include "OutputStringStream.h"

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationService::SerializationService(int version, std::map< int, PortableFactory const *  > const &portableFactories)
            : portableSerializer(this, portableFactories)
            , serializationContext(portableSerializer.getFactoryIds(), version, this) {

            };

            SerializationService::~SerializationService() {
                while (!outputPool.empty()) {
                    DataOutput *out = outputPool.front();
                    outputPool.pop();
                    delete out;
                }
            };

            void SerializationService::push(DataOutput *out) {
                out->reset();
                outputPool.push(out);
            };

            DataOutput *SerializationService::pop() {
                DataOutput *out;
                if (outputPool.empty()) {
                    out = new DataOutput(new OutputStringStream());
                } else {
                    out = outputPool.front();
                    outputPool.pop();
                }
                return out;
            };

            Data SerializationService::toData(bool object) {
                DataOutput *output = pop();
                booleanSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_BOOLEAN, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(char object) {
                DataOutput *output = pop();
                charSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_CHAR, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(short object) {
                DataOutput *output = pop();
                shortSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_SHORT, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(int object) {
                DataOutput *output = pop();
                integerSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_INTEGER, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(long object) {
                DataOutput *output = pop();
                longSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_LONG, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(float object) {
                DataOutput *output = pop();
                floatSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_FLOAT, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(double object) {
                DataOutput *output = pop();
                doubleSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_DOUBLE, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(std::vector<char>& object) {
                DataOutput *output = pop();
                charArraySerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_CHAR_ARRAY, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(std::vector<short>& object) {
                DataOutput *output = pop();
                shortArraySerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_SHORT_ARRAY, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(std::vector<int>& object) {
                DataOutput *output = pop();
                integerArraySerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_INTEGER_ARRAY, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(std::vector<long>& object) {
                DataOutput *output = pop();
                longArraySerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_LONG_ARRAY, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(std::vector<float>& object) {
                DataOutput *output = pop();
                floatArraySerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_FLOAT_ARRAY, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(std::vector<double>& object) {
                DataOutput *output = pop();
                doubleArraySerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_DOUBLE_ARRAY, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(string& object) {
                DataOutput *output = pop();
                stringSerializer.write(output, object);
                Data data(SerializationConstants::CONSTANT_TYPE_STRING, output->toByteArray());
                push(output);
                return data;
            };

            Data SerializationService::toData(Data& data) {
                return data;
            };

            long SerializationService::combineToLong(int x, int y) {
                return ((long) x << 32) | ((long) y & 0xFFFFFFFL);
            };

            int SerializationService::extractInt(long value, bool lowerBits) {
                return (lowerBits) ? (int) value : (int) (value >> 32);
            };

        }
    }
}