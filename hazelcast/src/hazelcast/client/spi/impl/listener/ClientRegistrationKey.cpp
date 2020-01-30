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

#include "hazelcast/client/spi/impl/listener/ClientRegistrationKey.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace listener {
                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId,
                                                                 const std::shared_ptr<EventHandler<protocol::ClientMessage> > &handler,
                                                                 const std::shared_ptr<ListenerMessageCodec> &codec)
                            : userRegistrationId(userRegistrationId), handler(handler), codec(codec) {
                    }

                    ClientRegistrationKey::ClientRegistrationKey(const std::string &userRegistrationId)
                            : userRegistrationId(userRegistrationId) {
                    }

                    const std::string &ClientRegistrationKey::getUserRegistrationId() const {
                        return userRegistrationId;
                    }

                    const std::shared_ptr<EventHandler<protocol::ClientMessage> > &
                    ClientRegistrationKey::getHandler() const {
                        return handler;
                    }

                    const std::shared_ptr<ListenerMessageCodec> &ClientRegistrationKey::getCodec() const {
                        return codec;
                    }

                    bool ClientRegistrationKey::operator==(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId == rhs.userRegistrationId;
                    }

                    bool ClientRegistrationKey::operator!=(const ClientRegistrationKey &rhs) const {
                        return !(rhs == *this);
                    }

                    bool ClientRegistrationKey::operator<(const ClientRegistrationKey &rhs) const {
                        return userRegistrationId < rhs.userRegistrationId;
                    }

                    std::ostream &operator<<(std::ostream &os, const ClientRegistrationKey &key) {
                        os << "ClientRegistrationKey{ userRegistrationId='" << key.userRegistrationId + '\'' + '}';
                        return os;
                    }

                    ClientRegistrationKey::ClientRegistrationKey() {}
                }
            }
        }

    }
}
