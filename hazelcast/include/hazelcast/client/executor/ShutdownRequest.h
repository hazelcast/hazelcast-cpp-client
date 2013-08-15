//
// Created by sancar koyunlu on 8/15/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_ShutdownRequest
#define HAZELCAST_ShutdownRequest


#include "IdentifiedDataSerializable.h"
#include "ObjectDataOutput.h"
#include "ObjectDataInput.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace executor {
            class ShutdownRequest : public IdentifiedDataSerializable {
            public:
                ShutdownRequest(const std::string& instanceName);

                int getFactoryId() const;

                int getClassId() const;

                void writeData(serialization::ObjectDataOutput& writer) const;

                void readData(serialization::ObjectDataInput& reader);
            private:
                const std::string& instanceName;

            };
        }
    }
}


#endif //HAZELCAST_ShutdownRequest
