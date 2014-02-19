//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PortableMessage
#define HAZELCAST_PortableMessage

#include "hazelcast/client/impl/PortableResponse.h"
#include "hazelcast/client/serialization/Data.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace topic {
            class HAZELCAST_API PortableMessage : public impl::PortableResponse {
            public:
                const serialization::Data& getMessage() const;

                std::string getUuid() const;

                long getPublishTime() const;

                int getFactoryId() const;

                int getClassId() const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::Data message;
                std::string uuid;
                long publishTime;
            };
        }
    }
}


#endif //HAZELCAST_PortableMessage
