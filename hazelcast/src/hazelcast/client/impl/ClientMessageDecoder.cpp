/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include <hazelcast/client/impl/ClientMessageDecoder.h>

#include "hazelcast/client/impl/ClientMessageDecoder.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            const boost::shared_ptr<ClientMessageDecoder<void> > VoidMessageDecoder::singleton(new VoidMessageDecoder);

            boost::shared_ptr<void>
            VoidMessageDecoder::decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                    serialization::pimpl::SerializationService &serializationService) {
                return boost::shared_ptr<void>();
            }

            const boost::shared_ptr<ClientMessageDecoder<void> > &VoidMessageDecoder::instance() {
                return singleton;
            }
        }
    }
}
