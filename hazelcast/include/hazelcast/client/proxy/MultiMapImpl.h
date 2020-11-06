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

#include <string>
#include <vector>
#include <chrono>

#include "hazelcast/client/proxy/ProxyImpl.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class ClientLockReferenceIdGenerator;
        }

        namespace proxy {
            class HAZELCAST_API MultiMapImpl : public ProxyImpl {
            public:
                /**
                * Returns the number of key-value pairs in the multimap.
                *
                * @return the number of key-value pairs in the multimap.
                */
                boost::future<int> size();

                /**
                * Clears the multimap. Removes all key-value pairs.
                */
                boost::future<void> clear();

                /**
                * Removes the specified entry listener
                * Returns silently if there is no such listener added before.
                *
                * @param registrationId Id of listener registration
                *
                * @return true if registration is removed, false otherwise
                */
                boost::future<bool> remove_entry_listener(boost::uuids::uuid registration_id);
            protected:
                MultiMapImpl(const std::string& instance_name, spi::ClientContext *context);

                boost::future<bool> put(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                boost::future<std::vector<serialization::pimpl::Data>> get_data(const serialization::pimpl::Data &key);

                boost::future<bool> remove(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                boost::future<std::vector<serialization::pimpl::Data>> remove_data(const serialization::pimpl::Data& key);

                boost::future<std::vector<serialization::pimpl::Data>> key_set_data();

                boost::future<std::vector<serialization::pimpl::Data>> values_data();

                boost::future<EntryVector> entry_set_data();

                boost::future<bool> contains_key(const serialization::pimpl::Data& key);

                boost::future<bool> contains_value(const serialization::pimpl::Data& key);

                boost::future<bool> contains_entry(const serialization::pimpl::Data& key, const serialization::pimpl::Data& value);

                boost::future<int> value_count(const serialization::pimpl::Data& key);

                boost::future<boost::uuids::uuid>
                add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler, bool include_value);

                boost::future<boost::uuids::uuid>
                add_entry_listener(std::shared_ptr<impl::BaseEventHandler> entry_event_handler, bool include_value,
                                 Data &&key);

                boost::future<void> lock(const serialization::pimpl::Data& key);

                boost::future<void> lock(const serialization::pimpl::Data& key, std::chrono::milliseconds lease_time);

                boost::future<bool> is_locked(const serialization::pimpl::Data& key);

                boost::future<bool> try_lock(const serialization::pimpl::Data& key);

                boost::future<bool> try_lock(const serialization::pimpl::Data& key, std::chrono::milliseconds timeout);

                boost::future<bool>
                try_lock(const serialization::pimpl::Data &key, std::chrono::milliseconds timeout,
                        std::chrono::milliseconds lease_time);

                boost::future<void> unlock(const serialization::pimpl::Data& key);

                boost::future<void> force_unlock(const serialization::pimpl::Data& key);

                void on_initialize() override;
            private:
                class MultiMapEntryListenerMessageCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MultiMapEntryListenerMessageCodec(std::string name, bool include_value);

                    protocol::ClientMessage encode_add_request(bool local_only) const override;

                    protocol::ClientMessage encode_remove_request(boost::uuids::uuid real_registration_id) const override;
                private:
                    std::string name_;
                    bool include_value_;
                };

                class MultiMapEntryListenerToKeyCodec : public spi::impl::ListenerMessageCodec {
                public:
                    MultiMapEntryListenerToKeyCodec(std::string name, bool include_value,
                                                    serialization::pimpl::Data &&key);

                    protocol::ClientMessage encode_add_request(bool local_only) const override;

                    protocol::ClientMessage encode_remove_request(boost::uuids::uuid real_registration_id) const override;
                private:
                    std::string  name_;
                    bool include_value_;
                    serialization::pimpl::Data key_;
                };

                std::shared_ptr<impl::ClientLockReferenceIdGenerator> lock_reference_id_generator_;

                std::shared_ptr<spi::impl::ListenerMessageCodec> create_multi_map_entry_listener_codec(bool include_value);

                std::shared_ptr<spi::impl::ListenerMessageCodec>
                create_multi_map_entry_listener_codec(bool include_value, serialization::pimpl::Data &&key);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

