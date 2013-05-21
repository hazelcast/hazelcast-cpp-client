//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_AUTHENTICATION_REQUEST
#define HAZELCAST_AUTHENTICATION_REQUEST

#include "Principal.h"
#include "Credentials.h"
#include "ProtocolConstants.h"
#include "../serialization/NullPortable.h"
#include "../serialization/Portable.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class AuthenticationRequest : public hazelcast::client::serialization::Portable {
                template<typename HzWriter>
                friend void hazelcast::client::serialization::writePortable(HzWriter& writer, const hazelcast::client::protocol::AuthenticationRequest& arr);

                template<typename HzReader>
                friend void hazelcast::client::serialization::readPortable(HzReader& reader, hazelcast::client::protocol::AuthenticationRequest& arr);

            public:
                AuthenticationRequest(Credentials credential);

                void setReAuth(bool);

            private:
                Credentials credentials;
                Principal principal;
                bool reAuth;
                bool isPrincipalSet;
            };

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {
            inline int getFactoryId(const hazelcast::client::protocol::AuthenticationRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            inline int getClassId(const hazelcast::client::protocol::AuthenticationRequest& ar) {
                return hazelcast::client::protocol::ProtocolConstants::AUTHENTICATION_REQUEST_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::protocol::AuthenticationRequest& arr) {
                writer["credentials"] << arr.credentials;
                if (!arr.isPrincipalSet) {
                    NullPortable nullPortable(-3, 3);
                    writer["principal"] << nullPortable;
                } else {
                    writer["principal"] << arr.principal;
                }
                writer["reAuth"] << arr.reAuth;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::protocol::AuthenticationRequest& arr) {
                reader["credentials"] >> arr.credentials;
                reader["principal"] >> arr.principal;
                reader["reAuth"] >> arr.reAuth;
            };

        }
    }
}
#endif //HAZELCAST_AUTHENTICATION_REQUEST
