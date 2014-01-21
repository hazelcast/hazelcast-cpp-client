//
// Created by sancar koyunlu on 25/12/13.
//


#ifndef HAZELCAST_ClientResponse
#define HAZELCAST_ClientResponse


#include "hazelcast/client/impl/IdentifiedDataSerializableResponse.h"
#include "hazelcast/client/serialization/Data.h"
#include "hazelcast/client/exception/ServerException.h"
#include <memory>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace connection {
            class HAZELCAST_API ClientResponse : public impl::IdentifiedDataSerializableResponse {
            public:
                const serialization::Data &getData() const;

                const exception::ServerException &getException() const;

                bool isEvent() const;

                bool isException() const;

                int getCallId() const;

                int getFactoryId() const;

                int getClassId() const;

                void readData(serialization::ObjectDataInput &reader);

            private:
                int callId;
                serialization::Data data;
                exception::ServerException error;
                bool event;
                bool exception;
            };

        }
    }
}

#endif //HAZELCAST_ClientResponse
