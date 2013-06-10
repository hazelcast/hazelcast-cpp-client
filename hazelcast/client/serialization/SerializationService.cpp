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
            : serializationContext(version)
            , portableSerializer(&serializationContext) {
            };

            SerializationService::SerializationService(SerializationService const & rhs)
            : serializationContext(1)
            , portableSerializer(&serializationContext) {
            };


            SerializationContext & SerializationService::getSerializationContext() {
                return serializationContext;
            };

            SerializationService::~SerializationService() {
            };


            Data SerializationService::toData(byte object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(bool object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(char object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(short object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(int object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(long object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(float object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(double object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(vector<byte> const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(vector<char> const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(vector<short> const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(vector<int> const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(vector<long> const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(vector<float> const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(vector<double> const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }

            Data SerializationService::toData(string const & object) {
                BufferedDataOutput output;
                writePortable(output, object);
                Data data;
                data.setBuffer(output.toByteArray());
                return data;
            }


        }
    }
}