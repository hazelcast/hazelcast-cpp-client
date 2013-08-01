//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_AUTHENTICATION_REQUEST
#define HAZELCAST_AUTHENTICATION_REQUEST

#include "Principal.h"
#include "Credentials.h"
#include "ProtocolConstants.h"
#include "Portable.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class AuthenticationRequest : public Portable {
            public:
                AuthenticationRequest(Credentials credential);

                void setPrincipal(Principal *principal);

                void setReAuth(bool);

                void setFirstConnection(bool);

                int getFactoryId() const;

                int getClassId() const;

                template<typename HzWriter>
                inline void writePortable(HzWriter& writer) const{
                    writer.writePortable("credentials", credentials);
                    if (principal == NULL) {
                        writer.writeNullPortable("principal",  -3, 3);
                    } else {
                        writer.writePortable("principal", *principal);
                    }
                    writer.writeBoolean("reAuth", reAuth);
                    writer.writeBoolean("firstConnection", firstConnection);
                };

                template<typename HzReader>
                inline void readPortable(HzReader& reader) {
                    credentials = reader.template readPortable<Credentials>("credentials");
                    protocol::Principal *principal = new protocol::Principal();
                    *principal = reader.template readPortable<Principal>("principal");
                    this->principal = principal;
                    reAuth = reader.readBoolean("reAuth");
                    firstConnection = reader.readBoolean("firstConnection");
                };

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
