//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.


#include "hazelcast/client/protocol/AuthenticationRequest.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
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

        }
    }
}