//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_AUTHENTICATION_REQUEST
#define HAZELCAST_AUTHENTICATION_REQUEST

#include "hazelcast/client/protocol/Principal.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/impl/PortableRequest.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class HAZELCAST_API AuthenticationRequest : public impl::PortableRequest {
            public:
                AuthenticationRequest(const Credentials &credential);

                void setPrincipal(Principal *principal);

                void setReAuth(bool);

                void setFirstConnection(bool);

                int getFactoryId() const;

                int getClassId() const;

                void write(serialization::PortableWriter &writer) const;

            private:
                const Credentials &credentials;
                Principal *principal;
                bool reAuth;
                bool firstConnection;
            };

        }
    }
}
#endif //HAZELCAST_AUTHENTICATION_REQUEST

