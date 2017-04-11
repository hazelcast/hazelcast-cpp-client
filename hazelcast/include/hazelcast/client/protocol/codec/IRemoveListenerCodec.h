/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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

/*
 *  Created on: 10.11.2015
 *      Author: ihsan
 */

#ifndef HAZELCAST_CLIENT_PROTOCOL_CODEC_IREMOVELISTENERCODEC_H_
#define HAZELCAST_CLIENT_PROTOCOL_CODEC_IREMOVELISTENERCODEC_H_

#include <memory>
#include <string>

#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;

            namespace codec {
                class HAZELCAST_API IRemoveListenerCodec {
                public:
                    virtual ~IRemoveListenerCodec() { }

                    virtual std::auto_ptr<ClientMessage> encodeRequest() const = 0;

                    virtual bool decodeResponse(ClientMessage &clientMessage) const = 0;

                    virtual const std::string &getRegistrationId() const = 0;

                    virtual void setRegistrationId(const std::string &id) = 0;
                };
            }
        }
    }
}

#endif /* HAZELCAST_CLIENT_PROTOCOL_CODEC_IREMOVELISTENERCODEC_H_ */
