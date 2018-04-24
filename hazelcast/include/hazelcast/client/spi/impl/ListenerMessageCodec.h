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

#ifndef HAZELCAST_CLIENT_SPI_IMPL_LISTENERMESSAGECODEC_H_
#define HAZELCAST_CLIENT_SPI_IMPL_LISTENERMESSAGECODEC_H_

#include <memory>
#include <string>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
        }
        namespace spi {
            namespace impl {
                class HAZELCAST_API ListenerMessageCodec {
                public:
                    virtual ~ListenerMessageCodec() {
                    }

                    virtual std::auto_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const = 0;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const = 0;

                    virtual std::auto_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const = 0;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const = 0;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_SPI_IMPL_LISTENERMESSAGECODEC_H_
