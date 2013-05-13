//
//  SerializationService.cpp
//  Server
//
//  Created by sancar koyunlu on 1/10/13.
//  Copyright (c) 2013 sancar koyunlu. All rights reserved.
//

#include "SerializationService.h"

using namespace std;

namespace hazelcast {
    namespace client {
        namespace serialization {

            SerializationService::SerializationService(int version)
            : serializationContext(version, this) {

            };

            SerializationService::SerializationService(SerializationService const & rhs)
            : serializationContext(1, this) {
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
                    out = new DataOutput();
                } else {
                    out = outputPool.front();
                    outputPool.pop();
                }
                return out;
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