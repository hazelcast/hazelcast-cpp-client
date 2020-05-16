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
//
// Created by sancar koyunlu on 01/10/14.
//
#pragma once
#include "hazelcast/client/proxy/ProxyImpl.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace proxy {
            class HAZELCAST_API ITopicImpl : public proxy::ProxyImpl {
            protected:
                ITopicImpl(const std::string& instanceName, spi::ClientContext *context);

                void publish(const serialization::pimpl::Data& data);

                std::string addMessageListener(impl::BaseEventHandler *topicEventHandler);

                virtual bool removeMessageListener(const std::string& registrationId);

            private:
                class TopicListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    TopicListenerMessageCodec(const std::string &name);

                    virtual std::unique_ptr<protocol::ClientMessage> encodeAddRequest(bool localOnly) const;

                    virtual std::string decodeAddResponse(protocol::ClientMessage &responseMessage) const;

                    virtual std::unique_ptr<protocol::ClientMessage>
                    encodeRemoveRequest(const std::string &realRegistrationId) const;

                    virtual bool decodeRemoveResponse(protocol::ClientMessage &clientMessage) const;

                private:
                    std::string name;
                };

                int partitionId;

                std::shared_ptr<spi::impl::ListenerMessageCodec> createItemListenerCodec();
            };
        }
    }
}

