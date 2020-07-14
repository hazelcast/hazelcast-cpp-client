/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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

#include "hazelcast/client/proxy/ProxyImpl.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API ITopicImpl : public proxy::ProxyImpl {
            public:
                /**
                * Stops receiving messages for the given message listener. If the given listener already removed,
                * this method does nothing.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> removeMessageListener(const std::string& registrationId);

            protected:
                ITopicImpl(const std::string& instanceName, spi::ClientContext *context);

                boost::future<void> publish(const serialization::pimpl::Data& data);

                boost::future<std::string> addMessageListener(std::unique_ptr<impl::BaseEventHandler> &&topicEventHandler);

            private:
                class TopicListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    TopicListenerMessageCodec(std::string name);

                    std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const override;

                    std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const override;

                    std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const override;

                    bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const override;

                private:
                    std::string name;
                };

                int partitionId;

                std::unique_ptr<spi::impl::ListenerMessageCodec> createItemListenerCodec();
            };
        }
    }
}
