//
// Created by sancar koyunlu on 6/25/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_PortableMessage
#define HAZELCAST_PortableMessage

#include "hazelcast/client/impl/PortableResponse.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace topic {
            class HAZELCAST_API PortableMessage : public impl::PortableResponse {
            public:
                const serialization::pimpl::Data& getMessage() const;

                std::string getUuid() const;

                long getPublishTime() const;

                int getFactoryId() const;

                int getClassId() const;

                void readPortable(serialization::PortableReader& reader);

            private:
                serialization::pimpl::Data message;
                std::string uuid;
                long publishTime;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_PortableMessage

