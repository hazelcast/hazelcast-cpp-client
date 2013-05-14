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

            SerializationService::~SerializationService() {
            };

            Data SerializationService::toData(Data& data) {
                return data;
            };


        }
    }
}