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
                boost::future<bool> remove_message_listener(boost::uuids::uuid registration_id);

            protected:
                ITopicImpl(const std::string& instance_name, spi::ClientContext *context);

                boost::future<void> publish(const serialization::pimpl::data& data);

                boost::future<boost::uuids::uuid> add_message_listener(std::shared_ptr<impl::BaseEventHandler> topic_event_handler);

            private:
                class TopicListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    TopicListenerMessageCodec(std::string name);

                    protocol::ClientMessage encode_add_request(bool local_only) const override;

                    protocol::ClientMessage encode_remove_request(boost::uuids::uuid real_registration_id) const override;

                private:
                    std::string name_;
                };

                int partition_id_;

                std::shared_ptr<spi::impl::ListenerMessageCodec> create_item_listener_codec();
            };
        }
    }
}
