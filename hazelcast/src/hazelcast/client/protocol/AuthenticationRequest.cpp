//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/protocol/AuthenticationRequest.h"
#include "PortableWriter.h"
#include "PortableReader.h"

namespace hazelcast {
    namespace client {
        namespace protocol {


            AuthenticationRequest::AuthenticationRequest() {

            };

            AuthenticationRequest::AuthenticationRequest(Credentials credentials)
            :credentials(credentials)
            , reAuth(true)
            , firstConnection(true) {

            };

            void AuthenticationRequest::setPrincipal(Principal *principal) {
                this->principal = principal;
            }

            void AuthenticationRequest::setReAuth(bool reAuth) {
                this->reAuth = reAuth;
            }

            void AuthenticationRequest::setFirstConnection(bool firstConnection) {
                this->firstConnection = firstConnection;
            }


            int AuthenticationRequest::getFactoryId() const {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            int AuthenticationRequest::getClassId() const {
                return protocol::ProtocolConstants::AUTHENTICATION_REQUEST_ID;
            };

            void AuthenticationRequest::writePortable(serialization::PortableWriter &writer) const {
                writer.writePortable("credentials", credentials);
                if (principal == NULL) {
                    writer.writeNullPortable("principal", -3, 3);
                } else {
                    writer.writePortable("principal", *principal);
                }
                writer.writeBoolean("reAuth", reAuth);
                writer.writeBoolean("firstConnection", firstConnection);
            };


            void AuthenticationRequest::readPortable(serialization::PortableReader &reader) {
                credentials = reader.readPortable<Credentials>("credentials");
                protocol::Principal *principal = new protocol::Principal();
                *principal = reader.readPortable<Principal>("principal");
                this->principal = principal;
                reAuth = reader.readBoolean("reAuth");
                firstConnection = reader.readBoolean("firstConnection");
            };

        }
    }
}