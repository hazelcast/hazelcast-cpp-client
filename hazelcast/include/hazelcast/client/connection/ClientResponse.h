//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_ClientResponse
#define HAZELCAST_ClientResponse


#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include "hazelcast/client/serialization/Data.h"
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace connection {
            class HAZELCAST_API ClientResponse : public impl::IdentifiedDataSerializableResponse {
            public:
                const serialization::Data& getData() const;

                bool isEvent() const;

                int getCallId() const;

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput &reader);

            private:
                int callId;
                serialization::Data data;
                bool event;
            };

        }
    }
}

#endif //HAZELCAST_ClientResponse
