//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_AddAndGetRequest
#define HAZELCAST_AddAndGetRequest

#include "AtomicLongRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class AddAndGetRequest : public AtomicLongRequest {
            public:
                AddAndGetRequest(const std::string& instanceName, long delta)
                : AtomicLongRequest(instanceName, delta) {

                };

                int getClassId() const {
                    return AtomicLongPortableHook::ADD_AND_GET;
                };


                void writePortable(serialization::PortableWriter& writer) const {
                    AtomicLongRequest::writePortable(writer);
                };


                void readPortable(serialization::PortableReader& reader) {
                    AtomicLongRequest::readPortable(reader);
                };

            };
        }
    }
}

#endif //HAZELCAST_AddAndGetRequest
