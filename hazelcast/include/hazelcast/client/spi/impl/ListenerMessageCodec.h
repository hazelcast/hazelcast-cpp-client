/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#pragma once

#include <boost/optional.hpp>
#include <boost/uuid/uuid.hpp>

#include "hazelcast/util/export.h"

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
                    virtual ~ListenerMessageCodec() = default;

                    virtual protocol::ClientMessage encode_add_request(bool local_only) const = 0;

                    boost::uuids::uuid decode_add_response(protocol::ClientMessage &msg) const;

                    virtual protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid real_registration_id) const = 0;

                    bool decode_remove_response(protocol::ClientMessage &msg) const;
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


