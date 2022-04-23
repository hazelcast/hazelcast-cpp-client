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

#include <unordered_map>
#include <boost/thread/future.hpp>

#include "hazelcast/client/serialization/serialization.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/entry_view.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#endif

namespace hazelcast {
namespace client {
namespace connection {
class Connection;
}
namespace spi {
class ClientContext;
namespace impl {
class ClientPartitionServiceImpl;
}
} // namespace spi
typedef std::vector<
  std::pair<serialization::pimpl::data, serialization::pimpl::data>>
  EntryVector;

namespace proxy {
/**
 * This base proxy can perform serialization and invocation related operations.
 */
class HAZELCAST_API SerializingProxy
{
public:
    template<typename T>
    static boost::future<void> to_void_future(boost::future<T> message_future)
    {
        return message_future.then(boost::launch::sync,
                                   [](boost::future<T> f) { f.get(); });
    }

protected:
    SerializingProxy(spi::ClientContext& context,
                     const std::string& object_name);

    boost::future<protocol::ClientMessage> invoke(
      protocol::ClientMessage& request);

    boost::future<protocol::ClientMessage> invoke_on_connection(
      protocol::ClientMessage& request,
      std::shared_ptr<connection::Connection> connection);

    boost::future<protocol::ClientMessage> invoke_on_partition(
      protocol::ClientMessage& request,
      int partition_id);

    boost::future<protocol::ClientMessage> invoke_on_key_owner(
      protocol::ClientMessage& request,
      const serialization::pimpl::data& key_data);

    boost::future<protocol::ClientMessage> invoke_on_member(
      protocol::ClientMessage& request,
      boost::uuids::uuid uuid);

    int get_partition_id(const serialization::pimpl::data& key);

    template<typename T>
    serialization::pimpl::data to_data(const T* object)
    {
        return serialization_service_.template to_data<T>(object);
    }

    template<typename T>
    serialization::pimpl::data to_data(const T& object)
    {
        return to_data<T>(&object);
    }

    template<typename FUNC>
    typename std::result_of<FUNC()>::type controlled_serialization(FUNC func)
    {
        using boost::future;
        using boost::launch;
        using serialization::pimpl::data;
        try {
            return func();
        } catch (exception::schema_not_replicated& exception) {
            return serialization_service_.get_compact_serializer()
              .replicate_schema(exception.schema())
              .then(boost::launch::sync,
                    [func](boost::future<void> f) {
                        f.get();
                        return func();
                    })
              .unwrap();
        }
    }

    template<typename K, typename V>
    inline boost::future<std::unordered_map<K, V>> to_object_map(
      std::vector<boost::future<EntryVector>>& futures)
    {
        return boost::when_all(futures.begin(), futures.end())
          .then(boost::launch::sync,
                [this](
                  boost::future<boost::csbl::vector<boost::future<EntryVector>>>
                    results_data) {
                    std::unordered_map<K, V> result;
                    for (auto& entryVectorFuture : results_data.get()) {
                        for (auto& entry : entryVectorFuture.get()) {
                            auto val = to_object<V>(entry.second);
                            // it is guaranteed that all values are non-null
                            assert(val.has_value());
                            result.emplace(to_object<K>(entry.first).value(),
                                           std::move(val.value()));
                        }
                    }
                    return result;
                });
    }

    template<typename T>
    inline boost::future<size_t> to_object_drain(
      boost::future<std::vector<serialization::pimpl::data>> f,
      std::vector<T>& elements)
    {
        return f.then(
          boost::launch::sync,
          [&elements,
           this](boost::future<std::vector<serialization::pimpl::data>> f) {
              auto datas = f.get();
              auto size = datas.size();
              elements.reserve(elements.size() + size);
              for (auto& data : datas) {
                  elements.push_back(to_object<T>(data).value());
              }
              return size;
          });
    }

    template<typename K, typename V>
    inline boost::future<std::vector<K>> keys_to_object_vector_with_predicate(
      boost::future<std::pair<std::vector<serialization::pimpl::data>,
                              query::anchor_data_list>> future,
      query::paging_predicate<K, V>& predicate)
    {
        return future.then(
          boost::launch::sync,
          [this, &predicate](
            boost::future<std::pair<std::vector<serialization::pimpl::data>,
                                    query::anchor_data_list>> f) {
              auto result = f.get();
              predicate.set_anchor_data_list(std::move(result.second));
              const auto& entries = result.first;
              std::vector<K> keys;
              keys.reserve(entries.size());
              for (const auto& e : entries) {
                  keys.emplace_back(*to_object<K>(e));
              }
              return keys;
          });
    }

    template<typename K, typename V>
    inline boost::future<std::vector<V>> values_to_object_vector_with_predicate(
      boost::future<std::pair<std::vector<serialization::pimpl::data>,
                              query::anchor_data_list>> future,
      query::paging_predicate<K, V>& predicate)
    {
        return future.then(
          boost::launch::sync,
          [this, &predicate](
            boost::future<std::pair<std::vector<serialization::pimpl::data>,
                                    query::anchor_data_list>> f) {
              auto result = f.get();
              predicate.set_anchor_data_list(std::move(result.second));
              const auto& entries = result.first;
              std::vector<V> values;
              values.reserve(entries.size());
              for (const auto& e : entries) {
                  values.emplace_back(*to_object<V>(e));
              }
              return values;
          });
    }

    template<typename K, typename V>
    inline boost::future<std::vector<std::pair<K, V>>>
    entry_set_to_object_vector_with_predicate(
      boost::future<std::pair<EntryVector, query::anchor_data_list>> future,
      query::paging_predicate<K, V>& predicate)
    {
        return future.then(
          boost::launch::sync,
          [this, &predicate](
            boost::future<std::pair<EntryVector, query::anchor_data_list>> f) {
              auto result = f.get();
              predicate.set_anchor_data_list(std::move(result.second));
              const auto& entries_data = result.first;
              std::vector<std::pair<K, V>> entries;
              entries.reserve(entries_data.size());
              for (const auto& e : entries_data) {
                  entries.emplace_back(*to_object<K>(e.first),
                                       *to_object<V>(e.second));
              }
              return entries;
          });
    }

    template<typename T>
    inline boost::future<boost::optional<T>> to_object(
      boost::future<boost::optional<serialization::pimpl::data>> f)
    {
        return f.then(
          boost::launch::sync,
          [this](boost::future<boost::optional<serialization::pimpl::data>> f) {
              return to_object<T>(f.get().get_ptr());
          });
    }

    template<typename T>
    boost::future<boost::optional<T>> to_object(
      boost::future<serialization::pimpl::data> f)
    {
        return f.then(boost::launch::sync,
                      [this](boost::future<serialization::pimpl::data> f) {
                          return to_object<T>(f.get());
                      });
    }

    template<typename T>
    boost::future<boost::optional<T>> to_object(
      boost::future<std::unique_ptr<serialization::pimpl::data>> f)
    {
        return f.then(
          boost::launch::sync,
          [this](boost::future<std::unique_ptr<serialization::pimpl::data>> f) {
              return to_object<T>(f.get());
          });
    }

    template<typename T>
    inline boost::future<std::vector<T>> to_object_vector(
      boost::future<std::vector<serialization::pimpl::data>> data_future)
    {
        return data_future.then(
          boost::launch::sync,
          [this](boost::future<std::vector<serialization::pimpl::data>> f) {
              auto dataResult = f.get();
              std::vector<T> result;
              result.reserve(dataResult.size());
              for (const auto& d : dataResult) {
                  // The object is guaranteed to exist (non-null)
                  result.push_back(std::move(to_object<T>(d).value()));
              }
              return result;
          });
    }

    template<typename K, typename V>
    boost::future<std::unordered_map<K, boost::optional<V>>> to_object_map(
      boost::future<EntryVector> entries_data)
    {
        return entries_data.then(
          boost::launch::sync, [this](boost::future<EntryVector> f) {
              auto entries = f.get();
              std::unordered_map<K, boost::optional<V>> result;
              result.reserve(entries.size());
              for (const auto& e : entries) {
                  result.insert({ std::move(to_object<K>(e.first)).value(),
                                  to_object<V>(e.second) });
              }
              return result;
          });
    }

    template<typename K, typename V>
    inline boost::future<boost::optional<entry_view<K, V>>>
    to_object_entry_view(
      boost::future<boost::optional<map::data_entry_view>> data_future)
    {
        return data_future.then(
          boost::launch::sync,
          [this](boost::future<boost::optional<map::data_entry_view>> f) {
              auto dataView = f.get();
              if (!dataView) {
                  return boost::optional<entry_view<K, V>>();
              }
              return boost::make_optional(
                entry_view<K, V>(to_object<K>(dataView->get_key()).value(),
                                 to_object<V>(dataView->get_value()).value(),
                                 dataView->get_cost(),
                                 dataView->get_creation_time(),
                                 dataView->get_expiration_time(),
                                 dataView->get_hits(),
                                 dataView->get_last_access_time(),
                                 dataView->get_last_stored_time(),
                                 dataView->get_last_update_time(),
                                 dataView->get_version()));
          });
    }

    template<typename K, typename V>
    inline boost::future<std::vector<std::pair<K, V>>> to_entry_object_vector(
      boost::future<EntryVector> data_future)
    {
        return data_future.then(
          boost::launch::sync, [this](boost::future<EntryVector> f) {
              auto dataEntryVector = f.get();
              std::vector<std::pair<K, V>> result;
              result.reserve(dataEntryVector.size());
              for (const auto& e : dataEntryVector) {
                  // please note that the key and value will never be null
                  result.emplace_back(
                    std::move(to_object<K>(e.first)).value(),
                    std::move(to_object<V>(e.second)).value());
              }
              return result;
          });
    }

    template<typename T>
    std::vector<serialization::pimpl::data> to_data_collection(
      const std::vector<T>& elements)
    {
        std::vector<serialization::pimpl::data> dataCollection;
        dataCollection.reserve(elements.size());
        for (const auto& e : elements) {
            dataCollection.push_back(to_data(e));
        }
        return dataCollection;
    }

    template<typename K, typename V>
    EntryVector to_data_entries(const std::unordered_map<K, V>& m)
    {
        EntryVector entries;
        entries.reserve(m.size());
        for (const auto& e : m) {
            entries.emplace_back(to_data<K>(e.first), to_data<V>(e.second));
        }
        return entries;
    }

    template<typename T>
    boost::future<T> invoke_and_get_future(protocol::ClientMessage& request)
    {
        return decode<T>(invoke(request));
    }

    template<typename T>
    boost::future<T> invoke_and_get_future(protocol::ClientMessage& request,
                                           int partition_id)
    {
        return decode<T>(invoke_on_partition(request, partition_id));
    }

    template<typename T>
    boost::future<T> invoke_and_get_future(
      protocol::ClientMessage& request,
      const serialization::pimpl::data& key)
    {
        return decode<T>(invoke_on_key_owner(request, key));
    }

protected:
    template<typename T>
    static boost::future<boost::optional<T>> decode_optional_var_sized(
      boost::future<protocol::ClientMessage> f)
    {
        return f.then(boost::launch::sync,
                      [](boost::future<protocol::ClientMessage> f) {
                          auto msg = f.get();
                          return msg.get_first_optional_var_sized_field<T>();
                      });
    }

    template<typename T>
    typename std::enable_if<std::is_trivial<T>::value, boost::future<T>>::
      type static decode(boost::future<protocol::ClientMessage> f)
    {
        return f.then(boost::launch::sync,
                      [](boost::future<protocol::ClientMessage> f) {
                          auto msg = f.get();
                          return msg.get_first_fixed_sized_field<T>();
                      });
    }

    template<typename T>
    typename std::enable_if<!std::is_trivial<T>::value, boost::future<T>>::
      type static decode(boost::future<protocol::ClientMessage> f)
    {
        return f.then(boost::launch::sync,
                      [](boost::future<protocol::ClientMessage> f) {
                          auto msg = f.get();
                          return *msg.get_first_var_sized_field<T>();
                      });
    }

    serialization::pimpl::SerializationService& serialization_service_;
    spi::impl::ClientPartitionServiceImpl& partition_service_;
    std::string object_name_;
    spi::ClientContext& client_context_;

private:
    template<typename T>
    typename std::enable_if<
      std::is_same<char*, typename std::remove_cv<T>::type>::value,
      boost::optional<std::string>>::
      type inline to_object(const serialization::pimpl::data& data)
    {
        return to_object<std::string>(data);
    }

    template<typename T>
    typename std::enable_if<
      std::is_array<T>::value &&
        std::is_same<typename std::remove_all_extents<T>::type, char>::value,
      boost::optional<std::string>>::
      type inline to_object(const serialization::pimpl::data& data)
    {
        return to_object<std::string>(data);
    }

    template<typename T>
    inline boost::optional<T> to_object(const serialization::pimpl::data& data)
    {
        return serialization_service_.template to_object<T>(data);
    }

    template<typename T>
    inline boost::optional<T> to_object(const serialization::pimpl::data* data)
    {
        if (!data) {
            return boost::none;
        } else {
            return to_object<T>(*data);
        }
    }

    template<typename T>
    inline boost::optional<T> to_object(
      std::unique_ptr<serialization::pimpl::data>&& data)
    {
        return to_object<T>(data.get());
    }

    template<typename T>
    inline boost::optional<T> to_object(
      std::unique_ptr<serialization::pimpl::data>& data)
    {
        return to_object<T>(data.get());
    }

    template<typename T>
    inline boost::optional<T> to_object(
      const boost::optional<serialization::pimpl::data>& data)
    {
        return to_object<T>(data.get_ptr());
    }
};

template<>
boost::future<boost::optional<serialization::pimpl::data>> HAZELCAST_API
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request);

template<>
boost::future<boost::optional<map::data_entry_view>> HAZELCAST_API
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request,
                                        const serialization::pimpl::data& key);

template<>
boost::future<boost::optional<serialization::pimpl::data>> HAZELCAST_API
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request,
                                        int partition_id);

template<>
boost::future<boost::optional<serialization::pimpl::data>> HAZELCAST_API
SerializingProxy::invoke_and_get_future(protocol::ClientMessage& request,
                                        const serialization::pimpl::data& key);

} // namespace proxy
} // namespace client
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
