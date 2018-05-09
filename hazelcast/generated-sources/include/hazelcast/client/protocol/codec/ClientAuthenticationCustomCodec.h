/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTAUTHENTICATIONCUSTOMCODEC_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTAUTHENTICATIONCUSTOMCODEC_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <memory>
#include <vector>
#include <string>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/protocol/codec/ClientMessageType.h"
#include "hazelcast/client/protocol/ResponseMessageConst.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"


using namespace hazelcast::client::serialization::pimpl;

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }
        class Address;

        class Member;

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ClientAuthenticationCustomCodec {
                public:
                    static const ClientMessageType REQUEST_TYPE;
                    static const bool RETRYABLE;
                    static const ResponseMessageConst RESPONSE_TYPE;

                    //************************ REQUEST STARTS ******************************************************************//
                    static std::auto_ptr<ClientMessage> encodeRequest(
                            const serialization::pimpl::Data &credentials,
                            const std::string *uuid,
                            const std::string *ownerUuid,
                            bool isOwnerConnection,
                            const std::string &clientType,
                            uint8_t serializationVersion,
                            const std::string &clientHazelcastVersion);

                    static int32_t calculateDataSize(
                            const serialization::pimpl::Data &credentials,
                            const std::string *uuid,
                            const std::string *ownerUuid,
                            bool isOwnerConnection,
                            const std::string &clientType,
                            uint8_t serializationVersion,
                            const std::string &clientHazelcastVersion);
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                    public:
                        uint8_t status;

                        std::auto_ptr<Address> address;

                        std::auto_ptr<std::string> uuid;

                        std::auto_ptr<std::string> ownerUuid;

                        uint8_t serializationVersion;

                        std::string serverHazelcastVersion;
                        bool serverHazelcastVersionExist;
                        std::auto_ptr<std::vector<Member> > clientUnregisteredMembers;
                        bool clientUnregisteredMembersExist;

                        static ResponseParameters decode(ClientMessage &clientMessage);

                        // define copy constructor (needed for auto_ptr variables)
                        ResponseParameters(const ResponseParameters &rhs);

                    private:
                        ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//

                private:
                    // Preventing public access to constructors
                    ClientAuthenticationCustomCodec();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_PROTOCOL_CODEC_CLIENTAUTHENTICATIONCUSTOMCODEC_H_ */

