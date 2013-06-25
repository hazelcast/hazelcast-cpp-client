//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAndSetRequest
#define HAZELCAST_GetAndSetRequest

#include "AtomicLongRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class GetAndSetRequest : public AtomicLongRequest {
            public:
                GetAndSetRequest(const std::string& instanceName, long value)
                : AtomicLongRequest(instanceName, value) {

                };

                int getClassId() const {
                    return AtomicLongPortableHook::GET_AND_SET;
                };

                template<typename HzWriter>
                void writePortable(HzWriter& writer) const {
                    AtomicLongRequest::writePortable(writer);
                };

                template<typename HzReader>
                void readPortable(HzReader& reader) {
                    AtomicLongRequest::readPortable(reader);
                };

            };
        }
    }
}


#endif //HAZELCAST_GetAndSetRequest
