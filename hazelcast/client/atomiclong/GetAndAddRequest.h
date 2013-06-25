//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_GetAndAddRequest
#define HAZELCAST_GetAndAddRequest

#include "AtomicLongRequest.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace atomiclong {

            class GetAndAddRequest : public AtomicLongRequest {
            public:
                GetAndAddRequest(const std::string& instanceName, long delta)
                : AtomicLongRequest(instanceName, delta) {

                };

                int getClassId() const {
                    return AtomicLongPortableHook::GET_AND_ADD;
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


#endif //HAZELCAST_GetAndAddRequest
