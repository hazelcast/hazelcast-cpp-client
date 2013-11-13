//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_IsShutDownRequest
#define HAZELCAST_IsShutDownRequest

#include "IdentifiedDataSerializable.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace executor {
            class IsShutdownRequest : public IdentifiedDataSerializable, public RetryableRequest {
            public:
                IsShutdownRequest(const std::string& instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);

            private:
                std::string instanceName;

            };
        }
    }
}

#endif //HAZELCAST_IsShutDownRequest

