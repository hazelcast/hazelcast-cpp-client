//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_AUTHENTICATION_REQUEST
#define HAZELCAST_AUTHENTICATION_REQUEST

#include "Principal.h"
#include "Credentials.h"
#include "ProtocolConstants.h"
#include "hazelcast/client/Portable.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class AuthenticationRequest : public Portable {
            public:
                AuthenticationRequest();

                AuthenticationRequest(Credentials credential);

                void setPrincipal(Principal *principal);

                void setReAuth(bool);

                void setFirstConnection(bool);

                int getFactoryId() const;

                int getClassId() const;

                void writePortable(serialization::PortableWriter &writer) const;

                void readPortable(serialization::PortableReader &reader);

            private:
                Credentials credentials;
                Principal *principal;
                bool reAuth;
                bool firstConnection;
            };

        }
    }
}
#endif //HAZELCAST_AUTHENTICATION_REQUEST
