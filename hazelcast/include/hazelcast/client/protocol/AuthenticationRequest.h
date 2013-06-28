//
// Created by sancar koyunlu on 5/7/13.
// Copyright (c) 2013 sancar koyunlu. All rights reserved.




#ifndef HAZELCAST_AUTHENTICATION_REQUEST
#define HAZELCAST_AUTHENTICATION_REQUEST

#include "Principal.h"
#include "Credentials.h"
#include "ProtocolConstants.h"
#include "../serialization/NullPortable.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class AuthenticationRequest {
                template<typename HzWriter>
                friend void serialization::writePortable(HzWriter& writer, const protocol::AuthenticationRequest& arr);

                template<typename HzReader>
                friend void serialization::readPortable(HzReader& reader, protocol::AuthenticationRequest& arr);

            public:
                AuthenticationRequest(Credentials credential);

                void setPrincipal(Principal *principal);

                void setReAuth(bool);

                void setFirstConnection(bool);

            private:
                Credentials credentials;
                Principal *principal;
                bool reAuth;
                bool firstConnection;
            };

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getTypeSerializerId(const protocol::AuthenticationRequest& x) {
                return SerializationConstants::CONSTANT_TYPE_PORTABLE;
            };

            inline int getFactoryId(const protocol::AuthenticationRequest& ar) {
                return protocol::ProtocolConstants::CLIENT_PORTABLE_FACTORY;
            }

            inline int getClassId(const protocol::AuthenticationRequest& ar) {
                return protocol::ProtocolConstants::AUTHENTICATION_REQUEST_ID;
            }


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const protocol::AuthenticationRequest& arr) {
                writer["credentials"] << arr.credentials;
                if (arr.principal == NULL) {
                    NullPortable nullPortable(-3, 3);
                    writer["principal"] << nullPortable;
                } else {
                    writer["principal"] << *arr.principal;
                }
                writer["reAuth"] << arr.reAuth;
                writer["firstConnection"] << arr.firstConnection;
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, protocol::AuthenticationRequest& arr) {
                reader["credentials"] >> arr.credentials;
                protocol::Principal *principal = new protocol::Principal();
                reader["principal"] >> (*principal);
                arr.principal = principal;
                reader["reAuth"] >> arr.reAuth;
                reader["firstConnection"] >> arr.firstConnection;
            };

        }
    }
}
#endif //HAZELCAST_AUTHENTICATION_REQUEST
