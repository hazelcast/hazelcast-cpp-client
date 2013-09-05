//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PublishRequest
#define HAZELCAST_PublishRequest

#include "Portable.h"
#include "Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class PublishRequest : public Portable {
            public:
                PublishRequest(const std::string& instanceName, const serialization::Data& message);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter& writer) const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data message;
                std::string instanceName;
            };
        }
    }
}


#endif //HAZELCAST_PublishRequest
