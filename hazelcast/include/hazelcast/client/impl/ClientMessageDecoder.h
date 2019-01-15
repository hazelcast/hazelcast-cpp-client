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

#ifndef HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_
#define HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_

#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/serialization/pimpl/Data.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            /**
             * Interface to pass specific decode methods to generic data structures that is dealing with client message
             */
            class ClientMessageDecoder {
            public:
                virtual boost::shared_ptr<serialization::pimpl::Data>
                decodeClientMessage(const boost::shared_ptr <protocol::ClientMessage> &clientMessage) = 0;
            };
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_

