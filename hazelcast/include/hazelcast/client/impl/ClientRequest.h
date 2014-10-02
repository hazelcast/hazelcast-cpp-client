//
// Created by sancar koyunlu on 9/11/13.
// Copyright (c) 2013 hazelcast. All rights reserved.




#ifndef HAZELCAST_Request
#define HAZELCAST_Request

#include "hazelcast/client/serialization/VersionedPortable.h"

namespace hazelcast {
    namespace client {
        namespace impl {

            class HAZELCAST_API ClientRequest : public serialization::VersionedPortable {
            public:
                ClientRequest();

                virtual ~ClientRequest();

                int getClassVersion() const;

                void writePortable(serialization::PortableWriter& writer) const;

                virtual void write(serialization::PortableWriter& writer) const = 0;

                /* final */ void readPortable(serialization::PortableReader& reader);

                virtual bool isRetryable() const;

                virtual bool isBindToSingleConnection() const;

                mutable int callId;
            };
        }
    }
}

#endif //HAZELCAST_Request

