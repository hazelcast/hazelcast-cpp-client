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
#include "hazelcast/client/spi/impl/ClientPartitionServiceImpl.h"
#include "hazelcast/client/spi/ClientContext.h"
#include "hazelcast/client/client_config.h"
#include "hazelcast/client/serialization/pimpl/data.h"
#include "hazelcast/client/internal/nearcache/NearCache.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace proxy {
class HAZELCAST_API ReplicatedMapImpl : public ProxyImpl
{
public:
    /**
     *
     * @return The number of the replicated map entries in the cluster.
     */
    boost::future<int32_t> size()
    {
        auto request = protocol::codec::replicatedmap_size_encode(name_);
        return invoke_and_get_future<int32_t>(request, target_partition_id_);
    }

    /**
     *
     * @return true if the replicated map is empty, false otherwise
     */
    boost::future<bool> is_empty()
    {
        auto request = protocol::codec::replicatedmap_isempty_encode(name_);
        return invoke_and_get_future<bool>(request, target_partition_id_);
    }

    /**
     * <p>The clear operation wipes data out of the replicated maps.
     * <p>If some node fails on executing the operation, it is retried for at
     * most 5 times (on the failing nodes only).
     */
    boost::future<void> clear()
    {
        try {
            auto request = protocol::codec::replicatedmap_clear_encode(name_);
            auto result = to_void_future(invoke(request));
            if (near_cache_) {
                near_cache_->clear();
            }
            return result;
        } catch (...) {
            if (near_cache_) {
                near_cache_->clear();
            }
            throw;
        }
    }

    /**
     * Removes the specified entry listener.
     * Returns silently if there was no such listener added before.
     *
     * @param registrationId ID of the registered entry listener.
     * @return true if registration is removed, false otherwise.
     */
    boost::future<bool> remove_entry_listener(
      boost::uuids::uuid registration_id)
    {
        return deregister_listener(registration_id);
    }

protected:
    boost::future<util::optional<serialization::pimpl::data>> put_data(
      serialization::pimpl::data&& key_data,
      serialization::pimpl::data&& value_data,
      std::chrono::milliseconds ttl)
    {
        try {
            auto request = protocol::codec::replicatedmap_put_encode(
              name_,
              key_data,
              value_data,
              std::chrono::duration_cast<std::chrono::milliseconds>(ttl)
                .count());
            auto result = invoke_and_get_future<
              util::optional<serialization::pimpl::data>>(request, key_data);

            invalidate(std::move(key_data));

            return result;
        } catch (...) {
            invalidate(std::move(key_data));
            throw;
        }
    }

    boost::future<util::optional<serialization::pimpl::data>> remove_data(
      serialization::pimpl::data&& key_data)
    {
        std::shared_ptr<serialization::pimpl::data> sharedKey(
          new serialization::pimpl::data(std::move(key_data)));
        try {
            auto request =
              protocol::codec::replicatedmap_remove_encode(name_, *sharedKey);
            auto result = invoke_and_get_future<
              util::optional<serialization::pimpl::data>>(request, *sharedKey);

            invalidate(sharedKey);

            return result;
        } catch (...) {
            invalidate(sharedKey);
            throw;
        }
    }

    virtual boost::future<void> put_all_data(EntryVector&& data_entries)
    {
        try {
            auto request =
              protocol::codec::replicatedmap_putall_encode(name_, data_entries);
            auto result = to_void_future(invoke(request));

            if (near_cache_) {
                for (auto& entry : data_entries) {
                    invalidate(std::move(entry.first));
                }
            }

            return result;
        } catch (...) {
            if (near_cache_) {
                for (auto& entry : data_entries) {
                    invalidate(std::move(entry.first));
                }
            }
            throw;
        }
    }

    boost::future<bool> contains_key_data(serialization::pimpl::data&& key_data)
    {
        auto request =
          protocol::codec::replicatedmap_containskey_encode(name_, key_data);
        return invoke_and_get_future<bool>(request, key_data);
    }

    boost::future<bool> contains_value_data(
      serialization::pimpl::data&& value_data)
    {
        auto request = protocol::codec::replicatedmap_containsvalue_encode(
          name_, value_data);
        return invoke_and_get_future<bool>(request, value_data);
    }

    boost::future<boost::uuids::uuid> add_entry_listener(
      std::shared_ptr<impl::BaseEventHandler> entry_event_handler)
    {
        return register_listener(create_entry_listener_codec(get_name()),
                                 std::move(entry_event_handler));
    }

    boost::future<boost::uuids::uuid> add_entry_listener_to_key(
      std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
      serialization::pimpl::data&& key)
    {
        return register_listener(
          create_entry_listener_to_key_codec(std::move(key)),
          std::move(entry_event_handler));
    }

    boost::future<boost::uuids::uuid> add_entry_listener(
      std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
      serialization::pimpl::data&& predicate)
    {
        return register_listener(
          create_entry_listener_with_predicate_codec(std::move(predicate)),
          std::move(entry_event_handler));
    }

    boost::future<boost::uuids::uuid> add_entry_listener(
      std::shared_ptr<impl::BaseEventHandler> entry_event_handler,
      serialization::pimpl::data&& key,
      serialization::pimpl::data&& predicate)
    {
        return register_listener(
          create_entry_listener_to_key_with_predicate_codec(
            std::move(key), std::move(predicate)),
          std::move(entry_event_handler));
    }

    boost::future<std::vector<serialization::pimpl::data>> values_data()
    {
        auto request = protocol::codec::replicatedmap_values_encode(get_name());
        return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
          request, target_partition_id_);
    }

    boost::future<EntryVector> entry_set_data()
    {
        auto request =
          protocol::codec::replicatedmap_entryset_encode(get_name());
        return invoke_and_get_future<EntryVector>(request,
                                                  target_partition_id_);
    }

    boost::future<std::vector<serialization::pimpl::data>> key_set_data()
    {
        auto request = protocol::codec::replicatedmap_keyset_encode(get_name());
        return invoke_and_get_future<std::vector<serialization::pimpl::data>>(
          request, target_partition_id_);
    }

    boost::future<util::optional<serialization::pimpl::data>> get_data(
      serialization::pimpl::data&& key)
    {
        auto sharedKey = std::make_shared<serialization::pimpl::data>(key);
        auto cachedValue = get_cached_data(sharedKey);
        if (cachedValue) {
            return boost::make_ready_future(boost::make_optional(*cachedValue));
        }
        auto request =
          protocol::codec::replicatedmap_get_encode(get_name(), *sharedKey);
        return invoke_and_get_future<
                 util::optional<serialization::pimpl::data>>(request, key)
          .then(
            boost::launch::sync,
            [=](boost::future<util::optional<serialization::pimpl::data>> f) {
                try {
                    auto response = f.get();
                    if (!response) {
                        return util::optional<serialization::pimpl::data>();
                    }

                    auto sharedValue =
                      std::make_shared<serialization::pimpl::data>(
                        std::move(*response));
                    if (near_cache_) {
                        near_cache_->put(sharedKey, sharedValue);
                    }
                    return boost::make_optional(*sharedValue);
                } catch (...) {
                    invalidate(sharedKey);
                    throw;
                }
            });
    }

    std::shared_ptr<serialization::pimpl::data> get_cached_data(
      const std::shared_ptr<serialization::pimpl::data>& key)
    {
        if (!near_cache_) {
            return nullptr;
        }
        return near_cache_->get(key);
    }

    void on_initialize() override
    {
        ProxyImpl::on_initialize();

        int partitionCount =
          get_context().get_partition_service().get_partition_count();
        target_partition_id_ = rand() % partitionCount;

        init_near_cache();
    }

    void post_destroy() override
    {
        try {
            if (near_cache_) {
                remove_near_cache_invalidation_listener();
                get_context().get_near_cache_manager().destroy_near_cache(
                  name_);
            }

            ProxyImpl::post_destroy();
        } catch (...) {
            ProxyImpl::post_destroy();
            throw;
        }
    }

    ReplicatedMapImpl(const std::string& service_name,
                      const std::string& object_name,
                      spi::ClientContext* context);

private:
    int target_partition_id_;
    std::shared_ptr<internal::nearcache::NearCache<serialization::pimpl::data,
                                                   serialization::pimpl::data>>
      near_cache_;
    boost::uuids::uuid invalidation_listener_id_;

    class NearCacheInvalidationListenerMessageCodec
      : public spi::impl::ListenerMessageCodec
    {
    public:
        NearCacheInvalidationListenerMessageCodec(const std::string& name)
          : name_(name)
        {}

        protocol::ClientMessage encode_add_request(
          bool local_only) const override
        {
            return protocol::codec::
              replicatedmap_addnearcacheentrylistener_encode(
                name_, false, local_only);
        }

        protocol::ClientMessage encode_remove_request(
          boost::uuids::uuid real_registration_id) const override
        {
            return protocol::codec::replicatedmap_removeentrylistener_encode(
              name_, real_registration_id);
        }

    private:
        std::string name_;
    };

    class ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec
      : public spi::impl::ListenerMessageCodec
    {
    public:
        ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(
          const std::string& name,
          serialization::pimpl::data&& key_data,
          serialization::pimpl::data&& predicate_data)
          : name_(name)
          , key_data_(key_data)
          , predicate_data_(predicate_data)
        {}

        protocol::ClientMessage encode_add_request(
          bool local_only) const override
        {
            return protocol::codec::
              replicatedmap_addentrylistenertokeywithpredicate_encode(
                name_, key_data_, predicate_data_, local_only);
        }

        protocol::ClientMessage encode_remove_request(
          boost::uuids::uuid real_registration_id) const override
        {
            return protocol::codec::replicatedmap_removeentrylistener_encode(
              name_, real_registration_id);
        }

    private:
        std::string name_;
        serialization::pimpl::data key_data_;
        serialization::pimpl::data predicate_data_;
    };

    class ReplicatedMapAddEntryListenerWithPredicateMessageCodec
      : public spi::impl::ListenerMessageCodec
    {
    public:
        ReplicatedMapAddEntryListenerWithPredicateMessageCodec(
          const std::string& name,
          serialization::pimpl::data&& key_data)
          : name_(name)
          , predicate_data_(key_data)
        {}

        protocol::ClientMessage encode_add_request(
          bool local_only) const override
        {
            return protocol::codec::
              replicatedmap_addentrylistenerwithpredicate_encode(
                name_, predicate_data_, local_only);
        }

        protocol::ClientMessage encode_remove_request(
          boost::uuids::uuid real_registration_id) const override
        {
            return protocol::codec::replicatedmap_removeentrylistener_encode(
              name_, real_registration_id);
        }

    private:
        std::string name_;
        serialization::pimpl::data predicate_data_;
    };

    class ReplicatedMapAddEntryListenerToKeyMessageCodec
      : public spi::impl::ListenerMessageCodec
    {
    public:
        ReplicatedMapAddEntryListenerToKeyMessageCodec(
          const std::string& name,
          serialization::pimpl::data&& key_data)
          : name_(name)
          , key_data_(key_data)
        {}

        protocol::ClientMessage encode_add_request(
          bool local_only) const override
        {
            return protocol::codec::replicatedmap_addentrylistenertokey_encode(
              name_, key_data_, local_only);
        }

        protocol::ClientMessage encode_remove_request(
          boost::uuids::uuid real_registration_id) const override
        {
            return protocol::codec::replicatedmap_removeentrylistener_encode(
              name_, real_registration_id);
        }

    private:
        std::string name_;
        serialization::pimpl::data key_data_;
    };

    class ReplicatedMapListenerMessageCodec
      : public spi::impl::ListenerMessageCodec
    {
    public:
        ReplicatedMapListenerMessageCodec(const std::string& name)
          : name_(name)
        {}

        protocol::ClientMessage encode_add_request(
          bool local_only) const override
        {
            return protocol::codec::replicatedmap_addentrylistener_encode(
              name_, local_only);
        }

        protocol::ClientMessage encode_remove_request(
          boost::uuids::uuid real_registration_id) const override
        {
            return protocol::codec::replicatedmap_removeentrylistener_encode(
              name_, real_registration_id);
        }

    private:
        std::string name_;
    };

    class ReplicatedMapAddNearCacheEventHandler
      : public protocol::codec::replicatedmap_addnearcacheentrylistener_handler
    {
    public:
        ReplicatedMapAddNearCacheEventHandler(
          const std::shared_ptr<
            internal::nearcache::NearCache<serialization::pimpl::data,
                                           serialization::pimpl::data>>&
            near_cache)
          : near_cache_(near_cache)
        {}

        void before_listener_register() override { near_cache_->clear(); }

        void on_listener_register() override { near_cache_->clear(); }

        void handle_entry(
          const util::optional<serialization::pimpl::data>& key,
          const util::optional<serialization::pimpl::data>& value,
          const util::optional<serialization::pimpl::data>& old_value,
          const util::optional<serialization::pimpl::data>& merging_value,
          int32_t event_type,
          boost::uuids::uuid uuid,
          int32_t number_of_affected_entries) override
        {
            switch (event_type) {
                case static_cast<int32_t>(entry_event::type::ADDED):
                case static_cast<int32_t>(entry_event::type::REMOVED):
                case static_cast<int32_t>(entry_event::type::UPDATED):
                case static_cast<int32_t>(entry_event::type::EVICTED): {
                    near_cache_->invalidate(
                      std::make_shared<serialization::pimpl::data>(
                        std::move(*key)));
                    break;
                }
                case static_cast<int32_t>(entry_event::type::CLEAR_ALL):
                    near_cache_->clear();
                    break;
                default:
                    BOOST_THROW_EXCEPTION(
                      (exception::exception_builder<
                         exception::illegal_argument>(
                         "ReplicatedMapAddNearCacheEventHandler::handle_entry")
                       << "Not a known event type " << event_type)
                        .build());
            }
        }

    private:
        std::shared_ptr<
          internal::nearcache::NearCache<serialization::pimpl::data,
                                         serialization::pimpl::data>>
          near_cache_;
    };

    std::shared_ptr<spi::impl::ListenerMessageCodec>
    create_entry_listener_codec(const std::string name)
    {
        return std::shared_ptr<spi::impl::ListenerMessageCodec>(
          new ReplicatedMapListenerMessageCodec(name));
    }

    std::shared_ptr<spi::impl::ListenerMessageCodec>
    create_entry_listener_to_key_codec(serialization::pimpl::data&& key_data)
    {
        return std::shared_ptr<spi::impl::ListenerMessageCodec>(
          new ReplicatedMapAddEntryListenerToKeyMessageCodec(
            name_, std::move(key_data)));
    }

    std::shared_ptr<spi::impl::ListenerMessageCodec>
    create_entry_listener_with_predicate_codec(
      serialization::pimpl::data&& predicate_data)
    {
        return std::shared_ptr<spi::impl::ListenerMessageCodec>(
          new ReplicatedMapAddEntryListenerWithPredicateMessageCodec(
            name_, std::move(predicate_data)));
    }

    std::shared_ptr<spi::impl::ListenerMessageCodec>
    create_entry_listener_to_key_with_predicate_codec(
      serialization::pimpl::data&& key_data,
      serialization::pimpl::data&& predicate_data)
    {
        return std::shared_ptr<spi::impl::ListenerMessageCodec>(
          new ReplicatedMapAddEntryListenerToKeyWithPredicateMessageCodec(
            name_, std::move(key_data), std::move(predicate_data)));
    }

    std::shared_ptr<spi::impl::ListenerMessageCodec>
    create_near_cache_invalidation_listener_codec()
    {
        return std::shared_ptr<spi::impl::ListenerMessageCodec>(
          new NearCacheInvalidationListenerMessageCodec(name_));
    }

    void register_invalidation_listener()
    {
        try {
            invalidation_listener_id_ =
              register_listener(
                create_near_cache_invalidation_listener_codec(),
                std::shared_ptr<impl::BaseEventHandler>(
                  new ReplicatedMapAddNearCacheEventHandler(near_cache_)))
                .get();
        } catch (exception::iexception& e) {
            HZ_LOG(get_context().get_logger(),
                   severe,
                   boost::str(boost::format("-----------------\n"
                                            "Near Cache is not initialized!\n"
                                            "-----------------"
                                            "%1%") %
                              e));
        }
    }

    void init_near_cache()
    {
        auto nearCacheConfig =
          get_context().get_client_config().get_near_cache_config(name_);
        if (nearCacheConfig) {
            near_cache_ =
              get_context()
                .get_near_cache_manager()
                .template get_or_create_near_cache<serialization::pimpl::data,
                                                   serialization::pimpl::data,
                                                   serialization::pimpl::data>(
                  name_, *nearCacheConfig);
            if (nearCacheConfig->is_invalidate_on_change()) {
                register_invalidation_listener();
            }
        }
    }

    void remove_near_cache_invalidation_listener()
    {
        if (near_cache_) {
            if (!invalidation_listener_id_.is_nil()) {
                deregister_listener(invalidation_listener_id_).get();
            }
        }
    }

    void invalidate(serialization::pimpl::data&& key)
    {
        if (!near_cache_) {
            return;
        }
        near_cache_->invalidate(
          std::make_shared<serialization::pimpl::data>(key));
    }

    void invalidate(const std::shared_ptr<serialization::pimpl::data>& key)
    {
        if (!near_cache_) {
            return;
        }
        near_cache_->invalidate(key);
    }
};
} // namespace proxy
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
