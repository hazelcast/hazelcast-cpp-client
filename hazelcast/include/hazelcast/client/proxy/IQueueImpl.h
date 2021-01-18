/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#include <vector>
#include <memory>
#include <chrono>
#include "hazelcast/client/proxy/ProxyImpl.h"
#include "hazelcast/client/impl/ItemEventHandler.h"

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl {
                class data;
            }
        }
        namespace proxy {
            class HAZELCAST_API IQueueImpl : public ProxyImpl {
            public:
                /**
                * Removes the specified item listener.
                * Returns silently if the specified listener is not added before.
                *
                * @param registrationId Id of listener registration.
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> remove_item_listener(
                        boost::uuids::uuid registration_id);

                /**
                *
                * @return size of this distributed queue
                */
                boost::future<int> size();

                /**
                *
                * @return true if queue is empty
                */
                boost::future<bool> is_empty();

                /**
                *
                * @return remaining capacity
                */
                boost::future<int> remaining_capacity();

                /**
                * Removes all elements from queue.
                */
                boost::future<void> clear();
            protected:
                IQueueImpl(const std::string& instance_name, spi::ClientContext *context);

                boost::future<boost::uuids::uuid>
                add_item_listener(std::unique_ptr<impl::item_event_handler<protocol::codec::queue_addlistener_handler>> &&item_event_handler, bool include_value) {
                    return register_listener(create_item_listener_codec(include_value), std::move(item_event_handler));
                }
                
                boost::future<bool> offer(const serialization::pimpl::data& element, std::chrono::milliseconds timeout);

                boost::future<void> put(const serialization::pimpl::data& element);

                boost::future<boost::optional<serialization::pimpl::data>> poll_data(std::chrono::milliseconds timeout);

                boost::future<bool> remove(const serialization::pimpl::data &element);

                boost::future<bool> contains(const serialization::pimpl::data &element);

                boost::future<std::vector<serialization::pimpl::data>> drain_to_data(size_t max_elements);

                boost::future<std::vector<serialization::pimpl::data>> drain_to_data();

                boost::future<boost::optional<serialization::pimpl::data>> take_data();

                boost::future<boost::optional<serialization::pimpl::data>> peek_data();

                boost::future<std::vector<serialization::pimpl::data>> to_array_data();

                boost::future<bool> contains_all_data(const std::vector<serialization::pimpl::data> &elements);

                boost::future<bool> add_all_data(const std::vector<serialization::pimpl::data> &elements);

                boost::future<bool> remove_all_data(const std::vector<serialization::pimpl::data> &elements);

                boost::future<bool> retain_all_data(const std::vector<serialization::pimpl::data>& elements);

            private:
                class QueueListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    QueueListenerMessageCodec(std::string name, bool include_value);

                    protocol::ClientMessage encode_add_request(bool local_only) const override;

                    protocol::ClientMessage
                    encode_remove_request(boost::uuids::uuid real_registration_id) const override;

                private:
                    std::string name_;
                    bool include_value_;
                };

                int partition_id_;

                std::shared_ptr<spi::impl::ListenerMessageCodec> create_item_listener_codec(bool include_value);
            };
        }
    }
}

