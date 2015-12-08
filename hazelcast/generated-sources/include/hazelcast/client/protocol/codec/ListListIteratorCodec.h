/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_LISTLISTITERATORCODEC_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_LISTLISTITERATORCODEC_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <memory>
#include <vector>
#include <string>


#include "hazelcast/client/protocol/codec/ListMessageType.h"
#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/impl/BaseEventHandler.h"
#include "hazelcast/client/protocol/ClientMessage.h"

#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {

        namespace protocol {
            namespace codec {
                class HAZELCAST_API ListListIteratorCodec {
                public:

                    //************************ REQUEST STARTS ******************************************************************//
                    class HAZELCAST_API RequestParameters {
                        public:
                            static const enum ListMessageType TYPE;
                            static const bool RETRYABLE;

                        static std::auto_ptr<ClientMessage> encode(
                                const std::string &name, 
                                int32_t index);

                        static int32_t calculateDataSize(
                                const std::string &name, 
                                int32_t index);

                        private:
                            // Preventing public access to constructors
                            RequestParameters();
                    };
                    //************************ REQUEST ENDS ********************************************************************//

                    //************************ RESPONSE STARTS *****************************************************************//
                    class HAZELCAST_API ResponseParameters {
                        public:
                            static const int TYPE;

                            std::vector<serialization::pimpl::Data > response;
                            
                            static ResponseParameters decode(ClientMessage &clientMessage);

                            // define copy constructor (needed for auto_ptr variables)
                            ResponseParameters(const ResponseParameters &rhs);
                        private:
                            ResponseParameters(ClientMessage &clientMessage);
                    };
                    //************************ RESPONSE ENDS *******************************************************************//
                    private:
                        // Preventing public access to constructors
                        ListListIteratorCodec ();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_PROTOCOL_CODEC_LISTLISTITERATORCODEC_H_ */

