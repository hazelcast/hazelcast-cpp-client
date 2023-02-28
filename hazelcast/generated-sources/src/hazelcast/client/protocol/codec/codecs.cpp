/*
 * Copyright (c) 2008-2023, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/uuid/uuid.hpp>
#include "hazelcast/client/member.h"
#include "hazelcast/client/serialization/pimpl/compact/schema.h"
#include "hazelcast/logger.h"

#include "codecs.h"

namespace hazelcast {
namespace client {
namespace protocol {
namespace codec {
ClientMessage
client_authentication_encode(const std::string& cluster_name,
                             const std::string* username,
                             const std::string* password,
                             boost::uuids::uuid uuid,
                             const std::string& client_type,
                             byte serialization_version,
                             const std::string& client_hazelcast_version,
                             const std::string& client_name,
                             const std::vector<std::string>& labels)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("client.authentication");

    msg.set_message_type(static_cast<int32_t>(256));
    msg.set_partition_id(-1);

    msg.set(uuid);
    msg.set(serialization_version);
    msg.set(cluster_name);

    msg.set_nullable(username);

    msg.set_nullable(password);

    msg.set(client_type);

    msg.set(client_hazelcast_version);

    msg.set(client_name);

    msg.set(labels, true);

    return msg;
}

ClientMessage
client_authenticationcustom_encode(const std::string& cluster_name,
                                   const std::vector<byte>& credentials,
                                   boost::uuids::uuid uuid,
                                   const std::string& client_type,
                                   byte serialization_version,
                                   const std::string& client_hazelcast_version,
                                   const std::string& client_name,
                                   const std::vector<std::string>& labels)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("client.authenticationcustom");

    msg.set_message_type(static_cast<int32_t>(512));
    msg.set_partition_id(-1);

    msg.set(uuid);
    msg.set(serialization_version);
    msg.set(cluster_name);

    msg.set(credentials);

    msg.set(client_type);

    msg.set(client_hazelcast_version);

    msg.set(client_name);

    msg.set(labels, true);

    return msg;
}

ClientMessage
client_addclusterviewlistener_encode()
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(false);
    msg.set_operation_name("client.addclusterviewlistener");

    msg.set_message_type(static_cast<int32_t>(768));
    msg.set_partition_id(-1);

    return msg;
}

void
client_addclusterviewlistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 770) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto version = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto member_infos = msg.get<std::vector<member>>();

        handle_membersview(version, member_infos);
        return;
    }
    if (messageType == 771) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto version = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto partitions = msg.get<
          std::vector<std::pair<boost::uuids::uuid, std::vector<int>>>>();
        handle_partitionsview(version, partitions);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[client_addclusterviewlistener_handler::handle] Unknown "
                     "message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
client_createproxy_encode(const std::string& name,
                          const std::string& service_name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("client.createproxy");

    msg.set_message_type(static_cast<int32_t>(1024));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(service_name, true);

    return msg;
}

ClientMessage
client_destroyproxy_encode(const std::string& name,
                           const std::string& service_name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("client.destroyproxy");

    msg.set_message_type(static_cast<int32_t>(1280));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(service_name, true);

    return msg;
}

ClientMessage
client_ping_encode()
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(true);
    msg.set_operation_name("client.ping");

    msg.set_message_type(static_cast<int32_t>(2816));
    msg.set_partition_id(-1);

    return msg;
}

ClientMessage
client_statistics_encode(int64_t timestamp,
                         const std::string& client_attributes,
                         const std::vector<byte>& metrics_blob)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("client.statistics");

    msg.set_message_type(static_cast<int32_t>(3072));
    msg.set_partition_id(-1);

    msg.set(timestamp);
    msg.set(client_attributes);

    msg.set(metrics_blob, true);

    return msg;
}

ClientMessage
client_localbackuplistener_encode()
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(false);
    msg.set_operation_name("client.localbackuplistener");

    msg.set_message_type(static_cast<int32_t>(3840));
    msg.set_partition_id(-1);

    return msg;
}

void
client_localbackuplistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 3842) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto source_invocation_correlation_id = msg.get<int64_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        handle_backup(source_invocation_correlation_id);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[client_localbackuplistener_handler::handle] Unknown "
                     "message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
map_put_encode(const std::string& name,
               const serialization::pimpl::data& key,
               const serialization::pimpl::data& value,
               int64_t thread_id,
               int64_t ttl)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.put");

    msg.set_message_type(static_cast<int32_t>(65792));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(ttl);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_get_encode(const std::string& name,
               const serialization::pimpl::data& key,
               int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.get");

    msg.set_message_type(static_cast<int32_t>(66048));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_remove_encode(const std::string& name,
                  const serialization::pimpl::data& key,
                  int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.remove");

    msg.set_message_type(static_cast<int32_t>(66304));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_replace_encode(const std::string& name,
                   const serialization::pimpl::data& key,
                   const serialization::pimpl::data& value,
                   int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.replace");

    msg.set_message_type(static_cast<int32_t>(66560));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_replaceifsame_encode(const std::string& name,
                         const serialization::pimpl::data& key,
                         const serialization::pimpl::data& test_value,
                         const serialization::pimpl::data& value,
                         int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.replaceifsame");

    msg.set_message_type(static_cast<int32_t>(66816));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(test_value);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_containskey_encode(const std::string& name,
                       const serialization::pimpl::data& key,
                       int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.containskey");

    msg.set_message_type(static_cast<int32_t>(67072));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_containsvalue_encode(const std::string& name,
                         const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.containsvalue");

    msg.set_message_type(static_cast<int32_t>(67328));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_removeifsame_encode(const std::string& name,
                        const serialization::pimpl::data& key,
                        const serialization::pimpl::data& value,
                        int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.removeifsame");

    msg.set_message_type(static_cast<int32_t>(67584));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_delete_encode(const std::string& name,
                  const serialization::pimpl::data& key,
                  int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.delete");

    msg.set_message_type(static_cast<int32_t>(67840));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_flush_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.flush");

    msg.set_message_type(static_cast<int32_t>(68096));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_tryremove_encode(const std::string& name,
                     const serialization::pimpl::data& key,
                     int64_t thread_id,
                     int64_t timeout)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.tryremove");

    msg.set_message_type(static_cast<int32_t>(68352));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(timeout);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_tryput_encode(const std::string& name,
                  const serialization::pimpl::data& key,
                  const serialization::pimpl::data& value,
                  int64_t thread_id,
                  int64_t timeout)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.tryput");

    msg.set_message_type(static_cast<int32_t>(68608));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(timeout);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_puttransient_encode(const std::string& name,
                        const serialization::pimpl::data& key,
                        const serialization::pimpl::data& value,
                        int64_t thread_id,
                        int64_t ttl)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.puttransient");

    msg.set_message_type(static_cast<int32_t>(68864));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(ttl);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_putifabsent_encode(const std::string& name,
                       const serialization::pimpl::data& key,
                       const serialization::pimpl::data& value,
                       int64_t thread_id,
                       int64_t ttl)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.putifabsent");

    msg.set_message_type(static_cast<int32_t>(69120));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(ttl);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_set_encode(const std::string& name,
               const serialization::pimpl::data& key,
               const serialization::pimpl::data& value,
               int64_t thread_id,
               int64_t ttl)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.set");

    msg.set_message_type(static_cast<int32_t>(69376));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(ttl);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
map_lock_encode(const std::string& name,
                const serialization::pimpl::data& key,
                int64_t thread_id,
                int64_t ttl,
                int64_t reference_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.lock");

    msg.set_message_type(static_cast<int32_t>(69632));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(ttl);
    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_trylock_encode(const std::string& name,
                   const serialization::pimpl::data& key,
                   int64_t thread_id,
                   int64_t lease,
                   int64_t timeout,
                   int64_t reference_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.trylock");

    msg.set_message_type(static_cast<int32_t>(69888));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(lease);
    msg.set(timeout);
    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_islocked_encode(const std::string& name,
                    const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.islocked");

    msg.set_message_type(static_cast<int32_t>(70144));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_unlock_encode(const std::string& name,
                  const serialization::pimpl::data& key,
                  int64_t thread_id,
                  int64_t reference_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.unlock");

    msg.set_message_type(static_cast<int32_t>(70400));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_addinterceptor_encode(const std::string& name,
                          const serialization::pimpl::data& interceptor)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.addinterceptor");

    msg.set_message_type(static_cast<int32_t>(70656));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(interceptor, true);

    return msg;
}

ClientMessage
map_removeinterceptor_encode(const std::string& name, const std::string& id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.removeinterceptor");

    msg.set_message_type(static_cast<int32_t>(70912));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(id, true);

    return msg;
}

ClientMessage
map_addentrylistenerwithpredicate_encode(
  const std::string& name,
  const serialization::pimpl::data& predicate,
  bool include_value,
  int32_t listener_flags,
  bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE +
      ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.addentrylistenerwithpredicate");

    msg.set_message_type(static_cast<int32_t>(71424));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(listener_flags);
    msg.set(local_only);
    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

void
map_addentrylistenerwithpredicate_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 71426) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[map_addentrylistenerwithpredicate_handler::handle] "
                     "Unknown message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
map_addentrylistenertokey_encode(const std::string& name,
                                 const serialization::pimpl::data& key,
                                 bool include_value,
                                 int32_t listener_flags,
                                 bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE +
      ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.addentrylistenertokey");

    msg.set_message_type(static_cast<int32_t>(71680));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(listener_flags);
    msg.set(local_only);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

void
map_addentrylistenertokey_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 71682) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format("[map_addentrylistenertokey_handler::handle] Unknown "
                          "message type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
map_addentrylistener_encode(const std::string& name,
                            bool include_value,
                            int32_t listener_flags,
                            bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE +
      ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.addentrylistener");

    msg.set_message_type(static_cast<int32_t>(71936));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(listener_flags);
    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
map_addentrylistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 71938) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format("[map_addentrylistener_handler::handle] Unknown "
                          "message type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
map_removeentrylistener_encode(const std::string& name,
                               boost::uuids::uuid registration_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.removeentrylistener");

    msg.set_message_type(static_cast<int32_t>(72192));
    msg.set_partition_id(-1);

    msg.set(registration_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
map_getentryview_encode(const std::string& name,
                        const serialization::pimpl::data& key,
                        int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.getentryview");

    msg.set_message_type(static_cast<int32_t>(72960));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_evict_encode(const std::string& name,
                 const serialization::pimpl::data& key,
                 int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.evict");

    msg.set_message_type(static_cast<int32_t>(73216));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_evictall_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.evictall");

    msg.set_message_type(static_cast<int32_t>(73472));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_keyset_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.keyset");

    msg.set_message_type(static_cast<int32_t>(74240));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_getall_encode(const std::string& name,
                  const std::vector<serialization::pimpl::data>& keys)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.getall");

    msg.set_message_type(static_cast<int32_t>(74496));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(keys, true);

    return msg;
}

ClientMessage
map_values_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.values");

    msg.set_message_type(static_cast<int32_t>(74752));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_entryset_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.entryset");

    msg.set_message_type(static_cast<int32_t>(75008));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_keysetwithpredicate_encode(const std::string& name,
                               const serialization::pimpl::data& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.keysetwithpredicate");

    msg.set_message_type(static_cast<int32_t>(75264));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_valueswithpredicate_encode(const std::string& name,
                               const serialization::pimpl::data& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.valueswithpredicate");

    msg.set_message_type(static_cast<int32_t>(75520));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_entrieswithpredicate_encode(const std::string& name,
                                const serialization::pimpl::data& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.entrieswithpredicate");

    msg.set_message_type(static_cast<int32_t>(75776));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_addindex_encode(const std::string& name,
                    const config::index_config& index_config)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.addindex");

    msg.set_message_type(static_cast<int32_t>(76032));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(index_config, true);

    return msg;
}

ClientMessage
map_size_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.size");

    msg.set_message_type(static_cast<int32_t>(76288));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_isempty_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.isempty");

    msg.set_message_type(static_cast<int32_t>(76544));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_putall_encode(
  const std::string& name,
  const std::vector<
    std::pair<serialization::pimpl::data, serialization::pimpl::data>>& entries,
  bool trigger_map_loader)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.putall");

    msg.set_message_type(static_cast<int32_t>(76800));
    msg.set_partition_id(-1);

    msg.set(trigger_map_loader);
    msg.set(name);

    msg.set(entries, true);

    return msg;
}

ClientMessage
map_clear_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.clear");

    msg.set_message_type(static_cast<int32_t>(77056));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
map_executeonkey_encode(const std::string& name,
                        const serialization::pimpl::data& entry_processor,
                        const serialization::pimpl::data& key,
                        int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.executeonkey");

    msg.set_message_type(static_cast<int32_t>(77312));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(entry_processor);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_submittokey_encode(const std::string& name,
                       const serialization::pimpl::data& entry_processor,
                       const serialization::pimpl::data& key,
                       int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.submittokey");

    msg.set_message_type(static_cast<int32_t>(77568));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(entry_processor);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_executeonallkeys_encode(const std::string& name,
                            const serialization::pimpl::data& entry_processor)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.executeonallkeys");

    msg.set_message_type(static_cast<int32_t>(77824));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(entry_processor, true);

    return msg;
}

ClientMessage
map_executewithpredicate_encode(
  const std::string& name,
  const serialization::pimpl::data& entry_processor,
  const serialization::pimpl::data& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.executewithpredicate");

    msg.set_message_type(static_cast<int32_t>(78080));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(entry_processor);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_executeonkeys_encode(const std::string& name,
                         const serialization::pimpl::data& entry_processor,
                         const std::vector<serialization::pimpl::data>& keys)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.executeonkeys");

    msg.set_message_type(static_cast<int32_t>(78336));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(entry_processor);

    msg.set(keys, true);

    return msg;
}

ClientMessage
map_forceunlock_encode(const std::string& name,
                       const serialization::pimpl::data& key,
                       int64_t reference_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.forceunlock");

    msg.set_message_type(static_cast<int32_t>(78592));
    msg.set_partition_id(-1);

    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
map_keysetwithpagingpredicate_encode(
  const std::string& name,
  const codec::holder::paging_predicate_holder& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.keysetwithpagingpredicate");

    msg.set_message_type(static_cast<int32_t>(78848));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_valueswithpagingpredicate_encode(
  const std::string& name,
  const codec::holder::paging_predicate_holder& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.valueswithpagingpredicate");

    msg.set_message_type(static_cast<int32_t>(79104));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_entrieswithpagingpredicate_encode(
  const std::string& name,
  const codec::holder::paging_predicate_holder& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.entrieswithpagingpredicate");

    msg.set_message_type(static_cast<int32_t>(79360));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_removeall_encode(const std::string& name,
                     const serialization::pimpl::data& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.removeall");

    msg.set_message_type(static_cast<int32_t>(81408));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
map_addnearcacheinvalidationlistener_encode(const std::string& name,
                                            int32_t listener_flags,
                                            bool local_only)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT32_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("map.addnearcacheinvalidationlistener");

    msg.set_message_type(static_cast<int32_t>(81664));
    msg.set_partition_id(-1);

    msg.set(listener_flags);
    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
map_addnearcacheinvalidationlistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 81666) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto source_uuid = msg.get<boost::uuids::uuid>();
        auto partition_uuid = msg.get<boost::uuids::uuid>();
        auto sequence = msg.get<int64_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        handle_imapinvalidation(key, source_uuid, partition_uuid, sequence);
        return;
    }
    if (messageType == 81667) {
        msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN);

        auto keys = msg.get<std::vector<serialization::pimpl::data>>();

        auto source_uuids = msg.get<std::vector<boost::uuids::uuid>>();
        auto partition_uuids = msg.get<std::vector<boost::uuids::uuid>>();
        auto sequences = msg.get<std::vector<int64_t>>();
        handle_imapbatchinvalidation(
          keys, source_uuids, partition_uuids, sequences);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[map_addnearcacheinvalidationlistener_handler::handle] "
                     "Unknown message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
map_replaceall_encode(const std::string& name,
                      const serialization::pimpl::data& function)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("map.replaceall");

    msg.set_message_type(static_cast<int32_t>(83968));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(function, true);

    return msg;
}

ClientMessage
multimap_put_encode(const std::string& name,
                    const serialization::pimpl::data& key,
                    const serialization::pimpl::data& value,
                    int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("multimap.put");

    msg.set_message_type(static_cast<int32_t>(131328));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
multimap_get_encode(const std::string& name,
                    const serialization::pimpl::data& key,
                    int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.get");

    msg.set_message_type(static_cast<int32_t>(131584));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_remove_encode(const std::string& name,
                       const serialization::pimpl::data& key,
                       int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("multimap.remove");

    msg.set_message_type(static_cast<int32_t>(131840));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_keyset_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.keyset");

    msg.set_message_type(static_cast<int32_t>(132096));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
multimap_values_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.values");

    msg.set_message_type(static_cast<int32_t>(132352));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
multimap_entryset_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.entryset");

    msg.set_message_type(static_cast<int32_t>(132608));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
multimap_containskey_encode(const std::string& name,
                            const serialization::pimpl::data& key,
                            int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.containskey");

    msg.set_message_type(static_cast<int32_t>(132864));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_containsvalue_encode(const std::string& name,
                              const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.containsvalue");

    msg.set_message_type(static_cast<int32_t>(133120));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
multimap_containsentry_encode(const std::string& name,
                              const serialization::pimpl::data& key,
                              const serialization::pimpl::data& value,
                              int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.containsentry");

    msg.set_message_type(static_cast<int32_t>(133376));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
multimap_size_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.size");

    msg.set_message_type(static_cast<int32_t>(133632));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
multimap_clear_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("multimap.clear");

    msg.set_message_type(static_cast<int32_t>(133888));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
multimap_valuecount_encode(const std::string& name,
                           const serialization::pimpl::data& key,
                           int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.valuecount");

    msg.set_message_type(static_cast<int32_t>(134144));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_addentrylistenertokey_encode(const std::string& name,
                                      const serialization::pimpl::data& key,
                                      bool include_value,
                                      bool local_only)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UINT8_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("multimap.addentrylistenertokey");

    msg.set_message_type(static_cast<int32_t>(134400));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(local_only);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

void
multimap_addentrylistenertokey_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 134402) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[multimap_addentrylistenertokey_handler::handle] Unknown "
                     "message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
multimap_addentrylistener_encode(const std::string& name,
                                 bool include_value,
                                 bool local_only)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UINT8_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("multimap.addentrylistener");

    msg.set_message_type(static_cast<int32_t>(134656));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
multimap_addentrylistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 134658) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format("[multimap_addentrylistener_handler::handle] Unknown "
                          "message type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
multimap_removeentrylistener_encode(const std::string& name,
                                    boost::uuids::uuid registration_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.removeentrylistener");

    msg.set_message_type(static_cast<int32_t>(134912));
    msg.set_partition_id(-1);

    msg.set(registration_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
multimap_lock_encode(const std::string& name,
                     const serialization::pimpl::data& key,
                     int64_t thread_id,
                     int64_t ttl,
                     int64_t reference_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.lock");

    msg.set_message_type(static_cast<int32_t>(135168));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(ttl);
    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_trylock_encode(const std::string& name,
                        const serialization::pimpl::data& key,
                        int64_t thread_id,
                        int64_t lease,
                        int64_t timeout,
                        int64_t reference_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.trylock");

    msg.set_message_type(static_cast<int32_t>(135424));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(lease);
    msg.set(timeout);
    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_islocked_encode(const std::string& name,
                         const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.islocked");

    msg.set_message_type(static_cast<int32_t>(135680));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_unlock_encode(const std::string& name,
                       const serialization::pimpl::data& key,
                       int64_t thread_id,
                       int64_t reference_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.unlock");

    msg.set_message_type(static_cast<int32_t>(135936));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_forceunlock_encode(const std::string& name,
                            const serialization::pimpl::data& key,
                            int64_t reference_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("multimap.forceunlock");

    msg.set_message_type(static_cast<int32_t>(136192));
    msg.set_partition_id(-1);

    msg.set(reference_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
multimap_removeentry_encode(const std::string& name,
                            const serialization::pimpl::data& key,
                            const serialization::pimpl::data& value,
                            int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("multimap.removeentry");

    msg.set_message_type(static_cast<int32_t>(136448));
    msg.set_partition_id(-1);

    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
queue_offer_encode(const std::string& name,
                   const serialization::pimpl::data& value,
                   int64_t timeout_millis)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.offer");

    msg.set_message_type(static_cast<int32_t>(196864));
    msg.set_partition_id(-1);

    msg.set(timeout_millis);
    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
queue_put_encode(const std::string& name,
                 const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.put");

    msg.set_message_type(static_cast<int32_t>(197120));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
queue_size_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.size");

    msg.set_message_type(static_cast<int32_t>(197376));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
queue_remove_encode(const std::string& name,
                    const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.remove");

    msg.set_message_type(static_cast<int32_t>(197632));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
queue_poll_encode(const std::string& name, int64_t timeout_millis)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.poll");

    msg.set_message_type(static_cast<int32_t>(197888));
    msg.set_partition_id(-1);

    msg.set(timeout_millis);
    msg.set(name, true);

    return msg;
}

ClientMessage
queue_take_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.take");

    msg.set_message_type(static_cast<int32_t>(198144));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
queue_peek_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.peek");

    msg.set_message_type(static_cast<int32_t>(198400));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
queue_iterator_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.iterator");

    msg.set_message_type(static_cast<int32_t>(198656));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
queue_drainto_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.drainto");

    msg.set_message_type(static_cast<int32_t>(198912));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
queue_draintomaxsize_encode(const std::string& name, int32_t max_size)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.draintomaxsize");

    msg.set_message_type(static_cast<int32_t>(199168));
    msg.set_partition_id(-1);

    msg.set(max_size);
    msg.set(name, true);

    return msg;
}

ClientMessage
queue_contains_encode(const std::string& name,
                      const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.contains");

    msg.set_message_type(static_cast<int32_t>(199424));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
queue_containsall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& data_list)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.containsall");

    msg.set_message_type(static_cast<int32_t>(199680));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(data_list, true);

    return msg;
}

ClientMessage
queue_compareandremoveall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& data_list)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.compareandremoveall");

    msg.set_message_type(static_cast<int32_t>(199936));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(data_list, true);

    return msg;
}

ClientMessage
queue_compareandretainall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& data_list)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.compareandretainall");

    msg.set_message_type(static_cast<int32_t>(200192));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(data_list, true);

    return msg;
}

ClientMessage
queue_clear_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.clear");

    msg.set_message_type(static_cast<int32_t>(200448));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
queue_addall_encode(const std::string& name,
                    const std::vector<serialization::pimpl::data>& data_list)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.addall");

    msg.set_message_type(static_cast<int32_t>(200704));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(data_list, true);

    return msg;
}

ClientMessage
queue_addlistener_encode(const std::string& name,
                         bool include_value,
                         bool local_only)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UINT8_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.addlistener");

    msg.set_message_type(static_cast<int32_t>(200960));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
queue_addlistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 200962) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto uuid = msg.get<boost::uuids::uuid>();
        auto event_type = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto item = msg.get_nullable<serialization::pimpl::data>();
        handle_item(item, uuid, event_type);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format("[queue_addlistener_handler::handle] Unknown message "
                          "type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
queue_removelistener_encode(const std::string& name,
                            boost::uuids::uuid registration_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("queue.removelistener");

    msg.set_message_type(static_cast<int32_t>(201216));
    msg.set_partition_id(-1);

    msg.set(registration_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
queue_remainingcapacity_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.remainingcapacity");

    msg.set_message_type(static_cast<int32_t>(201472));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
queue_isempty_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("queue.isempty");

    msg.set_message_type(static_cast<int32_t>(201728));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
topic_publish_encode(const std::string& name,
                     const serialization::pimpl::data& message)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("topic.publish");

    msg.set_message_type(static_cast<int32_t>(262400));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(message, true);

    return msg;
}

ClientMessage
topic_addmessagelistener_encode(const std::string& name, bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("topic.addmessagelistener");

    msg.set_message_type(static_cast<int32_t>(262656));
    msg.set_partition_id(-1);

    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
topic_addmessagelistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 262658) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto publish_time = msg.get<int64_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto item = msg.get<serialization::pimpl::data>();
        handle_topic(item, publish_time, uuid);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format("[topic_addmessagelistener_handler::handle] Unknown "
                          "message type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
topic_removemessagelistener_encode(const std::string& name,
                                   boost::uuids::uuid registration_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("topic.removemessagelistener");

    msg.set_message_type(static_cast<int32_t>(262912));
    msg.set_partition_id(-1);

    msg.set(registration_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
list_size_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.size");

    msg.set_message_type(static_cast<int32_t>(327936));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
list_contains_encode(const std::string& name,
                     const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.contains");

    msg.set_message_type(static_cast<int32_t>(328192));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
list_containsall_encode(const std::string& name,
                        const std::vector<serialization::pimpl::data>& values)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.containsall");

    msg.set_message_type(static_cast<int32_t>(328448));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(values, true);

    return msg;
}

ClientMessage
list_add_encode(const std::string& name,
                const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.add");

    msg.set_message_type(static_cast<int32_t>(328704));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
list_remove_encode(const std::string& name,
                   const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.remove");

    msg.set_message_type(static_cast<int32_t>(328960));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
list_addall_encode(const std::string& name,
                   const std::vector<serialization::pimpl::data>& value_list)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.addall");

    msg.set_message_type(static_cast<int32_t>(329216));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value_list, true);

    return msg;
}

ClientMessage
list_compareandremoveall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& values)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.compareandremoveall");

    msg.set_message_type(static_cast<int32_t>(329472));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(values, true);

    return msg;
}

ClientMessage
list_compareandretainall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& values)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.compareandretainall");

    msg.set_message_type(static_cast<int32_t>(329728));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(values, true);

    return msg;
}

ClientMessage
list_clear_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.clear");

    msg.set_message_type(static_cast<int32_t>(329984));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
list_getall_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.getall");

    msg.set_message_type(static_cast<int32_t>(330240));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
list_addlistener_encode(const std::string& name,
                        bool include_value,
                        bool local_only)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UINT8_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.addlistener");

    msg.set_message_type(static_cast<int32_t>(330496));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
list_addlistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 330498) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto uuid = msg.get<boost::uuids::uuid>();
        auto event_type = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto item = msg.get_nullable<serialization::pimpl::data>();
        handle_item(item, uuid, event_type);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format("[list_addlistener_handler::handle] Unknown message "
                          "type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
list_removelistener_encode(const std::string& name,
                           boost::uuids::uuid registration_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.removelistener");

    msg.set_message_type(static_cast<int32_t>(330752));
    msg.set_partition_id(-1);

    msg.set(registration_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
list_isempty_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.isempty");

    msg.set_message_type(static_cast<int32_t>(331008));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
list_addallwithindex_encode(
  const std::string& name,
  int32_t index,
  const std::vector<serialization::pimpl::data>& value_list)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.addallwithindex");

    msg.set_message_type(static_cast<int32_t>(331264));
    msg.set_partition_id(-1);

    msg.set(index);
    msg.set(name);

    msg.set(value_list, true);

    return msg;
}

ClientMessage
list_get_encode(const std::string& name, int32_t index)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.get");

    msg.set_message_type(static_cast<int32_t>(331520));
    msg.set_partition_id(-1);

    msg.set(index);
    msg.set(name, true);

    return msg;
}

ClientMessage
list_set_encode(const std::string& name,
                int32_t index,
                const serialization::pimpl::data& value)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.set");

    msg.set_message_type(static_cast<int32_t>(331776));
    msg.set_partition_id(-1);

    msg.set(index);
    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
list_addwithindex_encode(const std::string& name,
                         int32_t index,
                         const serialization::pimpl::data& value)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.addwithindex");

    msg.set_message_type(static_cast<int32_t>(332032));
    msg.set_partition_id(-1);

    msg.set(index);
    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
list_removewithindex_encode(const std::string& name, int32_t index)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("list.removewithindex");

    msg.set_message_type(static_cast<int32_t>(332288));
    msg.set_partition_id(-1);

    msg.set(index);
    msg.set(name, true);

    return msg;
}

ClientMessage
list_lastindexof_encode(const std::string& name,
                        const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.lastindexof");

    msg.set_message_type(static_cast<int32_t>(332544));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
list_indexof_encode(const std::string& name,
                    const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.indexof");

    msg.set_message_type(static_cast<int32_t>(332800));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
list_sub_encode(const std::string& name, int32_t from, int32_t to)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT32_SIZE +
                                ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("list.sub");

    msg.set_message_type(static_cast<int32_t>(333056));
    msg.set_partition_id(-1);

    msg.set(from);
    msg.set(to);
    msg.set(name, true);

    return msg;
}

ClientMessage
set_size_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.size");

    msg.set_message_type(static_cast<int32_t>(393472));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
set_contains_encode(const std::string& name,
                    const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.contains");

    msg.set_message_type(static_cast<int32_t>(393728));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
set_containsall_encode(const std::string& name,
                       const std::vector<serialization::pimpl::data>& items)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.containsall");

    msg.set_message_type(static_cast<int32_t>(393984));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(items, true);

    return msg;
}

ClientMessage
set_add_encode(const std::string& name, const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.add");

    msg.set_message_type(static_cast<int32_t>(394240));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
set_remove_encode(const std::string& name,
                  const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.remove");

    msg.set_message_type(static_cast<int32_t>(394496));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
set_addall_encode(const std::string& name,
                  const std::vector<serialization::pimpl::data>& value_list)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.addall");

    msg.set_message_type(static_cast<int32_t>(394752));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value_list, true);

    return msg;
}

ClientMessage
set_compareandremoveall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& values)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.compareandremoveall");

    msg.set_message_type(static_cast<int32_t>(395008));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(values, true);

    return msg;
}

ClientMessage
set_compareandretainall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& values)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.compareandretainall");

    msg.set_message_type(static_cast<int32_t>(395264));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(values, true);

    return msg;
}

ClientMessage
set_clear_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.clear");

    msg.set_message_type(static_cast<int32_t>(395520));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
set_getall_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.getall");

    msg.set_message_type(static_cast<int32_t>(395776));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
set_addlistener_encode(const std::string& name,
                       bool include_value,
                       bool local_only)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UINT8_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.addlistener");

    msg.set_message_type(static_cast<int32_t>(396032));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
set_addlistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 396034) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto uuid = msg.get<boost::uuids::uuid>();
        auto event_type = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto item = msg.get_nullable<serialization::pimpl::data>();
        handle_item(item, uuid, event_type);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format("[set_addlistener_handler::handle] Unknown message "
                          "type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
set_removelistener_encode(const std::string& name,
                          boost::uuids::uuid registration_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("set.removelistener");

    msg.set_message_type(static_cast<int32_t>(396288));
    msg.set_partition_id(-1);

    msg.set(registration_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
set_isempty_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("set.isempty");

    msg.set_message_type(static_cast<int32_t>(396544));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
fencedlock_lock_encode(const cp::raft_group_id& group_id,
                       const std::string& name,
                       int64_t session_id,
                       int64_t thread_id,
                       boost::uuids::uuid invocation_uid)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("fencedlock.lock");

    msg.set_message_type(static_cast<int32_t>(459008));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(thread_id);
    msg.set(invocation_uid);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
fencedlock_trylock_encode(const cp::raft_group_id& group_id,
                          const std::string& name,
                          int64_t session_id,
                          int64_t thread_id,
                          boost::uuids::uuid invocation_uid,
                          int64_t timeout_ms)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE +
      ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("fencedlock.trylock");

    msg.set_message_type(static_cast<int32_t>(459264));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(thread_id);
    msg.set(invocation_uid);
    msg.set(timeout_ms);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
fencedlock_unlock_encode(const cp::raft_group_id& group_id,
                         const std::string& name,
                         int64_t session_id,
                         int64_t thread_id,
                         boost::uuids::uuid invocation_uid)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("fencedlock.unlock");

    msg.set_message_type(static_cast<int32_t>(459520));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(thread_id);
    msg.set(invocation_uid);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
fencedlock_getlockownership_encode(const cp::raft_group_id& group_id,
                                   const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("fencedlock.getlockownership");

    msg.set_message_type(static_cast<int32_t>(459776));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
executorservice_shutdown_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("executorservice.shutdown");

    msg.set_message_type(static_cast<int32_t>(524544));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
executorservice_isshutdown_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("executorservice.isshutdown");

    msg.set_message_type(static_cast<int32_t>(524800));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
executorservice_cancelonpartition_encode(boost::uuids::uuid uuid,
                                         bool interrupt)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(false);
    msg.set_operation_name("executorservice.cancelonpartition");

    msg.set_message_type(static_cast<int32_t>(525056));
    msg.set_partition_id(-1);

    msg.set(uuid);
    msg.set(interrupt);
    return msg;
}

ClientMessage
executorservice_cancelonmember_encode(boost::uuids::uuid uuid,
                                      boost::uuids::uuid member_uuid,
                                      bool interrupt)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE +
      ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(false);
    msg.set_operation_name("executorservice.cancelonmember");

    msg.set_message_type(static_cast<int32_t>(525312));
    msg.set_partition_id(-1);

    msg.set(uuid);
    msg.set(member_uuid);
    msg.set(interrupt);
    return msg;
}

ClientMessage
executorservice_submittopartition_encode(
  const std::string& name,
  boost::uuids::uuid uuid,
  const serialization::pimpl::data& callable)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("executorservice.submittopartition");

    msg.set_message_type(static_cast<int32_t>(525568));
    msg.set_partition_id(-1);

    msg.set(uuid);
    msg.set(name);

    msg.set(callable, true);

    return msg;
}

ClientMessage
executorservice_submittomember_encode(
  const std::string& name,
  boost::uuids::uuid uuid,
  const serialization::pimpl::data& callable,
  boost::uuids::uuid member_uuid)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("executorservice.submittomember");

    msg.set_message_type(static_cast<int32_t>(525824));
    msg.set_partition_id(-1);

    msg.set(uuid);
    msg.set(member_uuid);
    msg.set(name);

    msg.set(callable, true);

    return msg;
}

ClientMessage
atomiclong_apply_encode(const cp::raft_group_id& group_id,
                        const std::string& name,
                        const serialization::pimpl::data& function)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomiclong.apply");

    msg.set_message_type(static_cast<int32_t>(590080));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name);

    msg.set(function, true);

    return msg;
}

ClientMessage
atomiclong_alter_encode(const cp::raft_group_id& group_id,
                        const std::string& name,
                        const serialization::pimpl::data& function,
                        int32_t return_value_type)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomiclong.alter");

    msg.set_message_type(static_cast<int32_t>(590336));
    msg.set_partition_id(-1);

    msg.set(return_value_type);
    msg.set(group_id);

    msg.set(name);

    msg.set(function, true);

    return msg;
}

ClientMessage
atomiclong_addandget_encode(const cp::raft_group_id& group_id,
                            const std::string& name,
                            int64_t delta)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomiclong.addandget");

    msg.set_message_type(static_cast<int32_t>(590592));
    msg.set_partition_id(-1);

    msg.set(delta);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
atomiclong_compareandset_encode(const cp::raft_group_id& group_id,
                                const std::string& name,
                                int64_t expected,
                                int64_t updated)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT64_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomiclong.compareandset");

    msg.set_message_type(static_cast<int32_t>(590848));
    msg.set_partition_id(-1);

    msg.set(expected);
    msg.set(updated);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
atomiclong_get_encode(const cp::raft_group_id& group_id,
                      const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("atomiclong.get");

    msg.set_message_type(static_cast<int32_t>(591104));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
atomiclong_getandadd_encode(const cp::raft_group_id& group_id,
                            const std::string& name,
                            int64_t delta)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomiclong.getandadd");

    msg.set_message_type(static_cast<int32_t>(591360));
    msg.set_partition_id(-1);

    msg.set(delta);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
atomiclong_getandset_encode(const cp::raft_group_id& group_id,
                            const std::string& name,
                            int64_t new_value)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomiclong.getandset");

    msg.set_message_type(static_cast<int32_t>(591616));
    msg.set_partition_id(-1);

    msg.set(new_value);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
atomicref_apply_encode(const cp::raft_group_id& group_id,
                       const std::string& name,
                       const serialization::pimpl::data& function,
                       int32_t return_value_type,
                       bool alter)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::INT32_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomicref.apply");

    msg.set_message_type(static_cast<int32_t>(655616));
    msg.set_partition_id(-1);

    msg.set(return_value_type);
    msg.set(alter);
    msg.set(group_id);

    msg.set(name);

    msg.set(function, true);

    return msg;
}

ClientMessage
atomicref_compareandset_encode(const cp::raft_group_id& group_id,
                               const std::string& name,
                               const serialization::pimpl::data* old_value,
                               const serialization::pimpl::data* new_value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomicref.compareandset");

    msg.set_message_type(static_cast<int32_t>(655872));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name);

    msg.set_nullable(old_value);

    msg.set_nullable(new_value, true);

    return msg;
}

ClientMessage
atomicref_contains_encode(const cp::raft_group_id& group_id,
                          const std::string& name,
                          const serialization::pimpl::data* value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("atomicref.contains");

    msg.set_message_type(static_cast<int32_t>(656128));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name);

    msg.set_nullable(value, true);

    return msg;
}

ClientMessage
atomicref_get_encode(const cp::raft_group_id& group_id, const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("atomicref.get");

    msg.set_message_type(static_cast<int32_t>(656384));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
atomicref_set_encode(const cp::raft_group_id& group_id,
                     const std::string& name,
                     const serialization::pimpl::data* new_value,
                     bool return_old_value)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("atomicref.set");

    msg.set_message_type(static_cast<int32_t>(656640));
    msg.set_partition_id(-1);

    msg.set(return_old_value);
    msg.set(group_id);

    msg.set(name);

    msg.set_nullable(new_value, true);

    return msg;
}

ClientMessage
countdownlatch_trysetcount_encode(const cp::raft_group_id& group_id,
                                  const std::string& name,
                                  int32_t count)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("countdownlatch.trysetcount");

    msg.set_message_type(static_cast<int32_t>(721152));
    msg.set_partition_id(-1);

    msg.set(count);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
countdownlatch_await_encode(const cp::raft_group_id& group_id,
                            const std::string& name,
                            boost::uuids::uuid invocation_uid,
                            int64_t timeout_ms)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("countdownlatch.await");

    msg.set_message_type(static_cast<int32_t>(721408));
    msg.set_partition_id(-1);

    msg.set(invocation_uid);
    msg.set(timeout_ms);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
countdownlatch_countdown_encode(const cp::raft_group_id& group_id,
                                const std::string& name,
                                boost::uuids::uuid invocation_uid,
                                int32_t expected_round)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("countdownlatch.countdown");

    msg.set_message_type(static_cast<int32_t>(721664));
    msg.set_partition_id(-1);

    msg.set(invocation_uid);
    msg.set(expected_round);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
countdownlatch_getcount_encode(const cp::raft_group_id& group_id,
                               const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("countdownlatch.getcount");

    msg.set_message_type(static_cast<int32_t>(721920));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
countdownlatch_getround_encode(const cp::raft_group_id& group_id,
                               const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("countdownlatch.getround");

    msg.set_message_type(static_cast<int32_t>(722176));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
semaphore_init_encode(const cp::raft_group_id& group_id,
                      const std::string& name,
                      int32_t permits)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("semaphore.init");

    msg.set_message_type(static_cast<int32_t>(786688));
    msg.set_partition_id(-1);

    msg.set(permits);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
semaphore_acquire_encode(const cp::raft_group_id& group_id,
                         const std::string& name,
                         int64_t session_id,
                         int64_t thread_id,
                         boost::uuids::uuid invocation_uid,
                         int32_t permits,
                         int64_t timeout_ms)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE +
      ClientMessage::INT32_SIZE + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("semaphore.acquire");

    msg.set_message_type(static_cast<int32_t>(786944));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(thread_id);
    msg.set(invocation_uid);
    msg.set(permits);
    msg.set(timeout_ms);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
semaphore_release_encode(const cp::raft_group_id& group_id,
                         const std::string& name,
                         int64_t session_id,
                         int64_t thread_id,
                         boost::uuids::uuid invocation_uid,
                         int32_t permits)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE +
      ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("semaphore.release");

    msg.set_message_type(static_cast<int32_t>(787200));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(thread_id);
    msg.set(invocation_uid);
    msg.set(permits);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
semaphore_drain_encode(const cp::raft_group_id& group_id,
                       const std::string& name,
                       int64_t session_id,
                       int64_t thread_id,
                       boost::uuids::uuid invocation_uid)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("semaphore.drain");

    msg.set_message_type(static_cast<int32_t>(787456));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(thread_id);
    msg.set(invocation_uid);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
semaphore_change_encode(const cp::raft_group_id& group_id,
                        const std::string& name,
                        int64_t session_id,
                        int64_t thread_id,
                        boost::uuids::uuid invocation_uid,
                        int32_t permits)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE +
      ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("semaphore.change");

    msg.set_message_type(static_cast<int32_t>(787712));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(thread_id);
    msg.set(invocation_uid);
    msg.set(permits);
    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
semaphore_availablepermits_encode(const cp::raft_group_id& group_id,
                                  const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("semaphore.availablepermits");

    msg.set_message_type(static_cast<int32_t>(787968));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(name, true);

    return msg;
}

ClientMessage
semaphore_getsemaphoretype_encode(const std::string& proxy_name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("semaphore.getsemaphoretype");

    msg.set_message_type(static_cast<int32_t>(788224));
    msg.set_partition_id(-1);

    msg.set(proxy_name, true);

    return msg;
}

ClientMessage
replicatedmap_put_encode(const std::string& name,
                         const serialization::pimpl::data& key,
                         const serialization::pimpl::data& value,
                         int64_t ttl)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.put");

    msg.set_message_type(static_cast<int32_t>(852224));
    msg.set_partition_id(-1);

    msg.set(ttl);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
replicatedmap_size_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.size");

    msg.set_message_type(static_cast<int32_t>(852480));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
replicatedmap_isempty_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.isempty");

    msg.set_message_type(static_cast<int32_t>(852736));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
replicatedmap_containskey_encode(const std::string& name,
                                 const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.containskey");

    msg.set_message_type(static_cast<int32_t>(852992));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
replicatedmap_containsvalue_encode(const std::string& name,
                                   const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.containsvalue");

    msg.set_message_type(static_cast<int32_t>(853248));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
replicatedmap_get_encode(const std::string& name,
                         const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.get");

    msg.set_message_type(static_cast<int32_t>(853504));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
replicatedmap_remove_encode(const std::string& name,
                            const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.remove");

    msg.set_message_type(static_cast<int32_t>(853760));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
replicatedmap_putall_encode(
  const std::string& name,
  const std::vector<
    std::pair<serialization::pimpl::data, serialization::pimpl::data>>& entries)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.putall");

    msg.set_message_type(static_cast<int32_t>(854016));
    msg.set_partition_id(-1);

    msg.set(name);

    msg.set(entries, true);

    return msg;
}

ClientMessage
replicatedmap_clear_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.clear");

    msg.set_message_type(static_cast<int32_t>(854272));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
replicatedmap_addentrylistenertokeywithpredicate_encode(
  const std::string& name,
  const serialization::pimpl::data& key,
  const serialization::pimpl::data& predicate,
  bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.addentrylistenertokeywithpredicate");

    msg.set_message_type(static_cast<int32_t>(854528));
    msg.set_partition_id(-1);

    msg.set(local_only);
    msg.set(name);

    msg.set(key);

    msg.set(predicate, true);

    return msg;
}

void
replicatedmap_addentrylistenertokeywithpredicate_handler::handle(
  ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 854530) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format(
              "[replicatedmap_addentrylistenertokeywithpredicate_handler::"
              "handle] Unknown message type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
replicatedmap_addentrylistenerwithpredicate_encode(
  const std::string& name,
  const serialization::pimpl::data& predicate,
  bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.addentrylistenerwithpredicate");

    msg.set_message_type(static_cast<int32_t>(854784));
    msg.set_partition_id(-1);

    msg.set(local_only);
    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

void
replicatedmap_addentrylistenerwithpredicate_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 854786) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(*get_logger(),
           warning,
           (boost::format(
              "[replicatedmap_addentrylistenerwithpredicate_handler::handle] "
              "Unknown message type (%1%) received on event handler.") %
            messageType)
             .str());
}

ClientMessage
replicatedmap_addentrylistenertokey_encode(
  const std::string& name,
  const serialization::pimpl::data& key,
  bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.addentrylistenertokey");

    msg.set_message_type(static_cast<int32_t>(855040));
    msg.set_partition_id(-1);

    msg.set(local_only);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

void
replicatedmap_addentrylistenertokey_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 855042) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[replicatedmap_addentrylistenertokey_handler::handle] "
                     "Unknown message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
replicatedmap_addentrylistener_encode(const std::string& name, bool local_only)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.addentrylistener");

    msg.set_message_type(static_cast<int32_t>(855296));
    msg.set_partition_id(-1);

    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
replicatedmap_addentrylistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 855298) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[replicatedmap_addentrylistener_handler::handle] Unknown "
                     "message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
replicatedmap_removeentrylistener_encode(const std::string& name,
                                         boost::uuids::uuid registration_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.removeentrylistener");

    msg.set_message_type(static_cast<int32_t>(855552));
    msg.set_partition_id(-1);

    msg.set(registration_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
replicatedmap_keyset_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.keyset");

    msg.set_message_type(static_cast<int32_t>(855808));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
replicatedmap_values_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.values");

    msg.set_message_type(static_cast<int32_t>(856064));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
replicatedmap_entryset_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("replicatedmap.entryset");

    msg.set_message_type(static_cast<int32_t>(856320));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
replicatedmap_addnearcacheentrylistener_encode(const std::string& name,
                                               bool include_value,
                                               bool local_only)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UINT8_SIZE +
                                ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("replicatedmap.addnearcacheentrylistener");

    msg.set_message_type(static_cast<int32_t>(856576));
    msg.set_partition_id(-1);

    msg.set(include_value);
    msg.set(local_only);
    msg.set(name, true);

    return msg;
}

void
replicatedmap_addnearcacheentrylistener_handler::handle(ClientMessage& msg)
{
    auto messageType = msg.get_message_type();
    if (messageType == 856578) {
        auto* initial_frame = reinterpret_cast<ClientMessage::frame_header_type*>(
          msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
        auto event_type = msg.get<int32_t>();
        auto uuid = msg.get<boost::uuids::uuid>();
        auto number_of_affected_entries = msg.get<int32_t>();
        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

        auto key = msg.get_nullable<serialization::pimpl::data>();
        auto value = msg.get_nullable<serialization::pimpl::data>();
        auto old_value = msg.get_nullable<serialization::pimpl::data>();
        auto merging_value = msg.get_nullable<serialization::pimpl::data>();
        handle_entry(key,
                     value,
                     old_value,
                     merging_value,
                     event_type,
                     uuid,
                     number_of_affected_entries);
        return;
    }
    HZ_LOG(
      *get_logger(),
      warning,
      (boost::format("[replicatedmap_addnearcacheentrylistener_handler::handle]"
                     " Unknown message type (%1%) received on event handler.") %
       messageType)
        .str());
}

ClientMessage
transactionalmap_containskey_encode(const std::string& name,
                                    boost::uuids::uuid txn_id,
                                    int64_t thread_id,
                                    const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.containskey");

    msg.set_message_type(static_cast<int32_t>(917760));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
transactionalmap_get_encode(const std::string& name,
                            boost::uuids::uuid txn_id,
                            int64_t thread_id,
                            const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.get");

    msg.set_message_type(static_cast<int32_t>(918016));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
transactionalmap_size_encode(const std::string& name,
                             boost::uuids::uuid txn_id,
                             int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.size");

    msg.set_message_type(static_cast<int32_t>(918528));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionalmap_isempty_encode(const std::string& name,
                                boost::uuids::uuid txn_id,
                                int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.isempty");

    msg.set_message_type(static_cast<int32_t>(918784));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionalmap_put_encode(const std::string& name,
                            boost::uuids::uuid txn_id,
                            int64_t thread_id,
                            const serialization::pimpl::data& key,
                            const serialization::pimpl::data& value,
                            int64_t ttl)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.put");

    msg.set_message_type(static_cast<int32_t>(919040));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(ttl);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
transactionalmap_set_encode(const std::string& name,
                            boost::uuids::uuid txn_id,
                            int64_t thread_id,
                            const serialization::pimpl::data& key,
                            const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.set");

    msg.set_message_type(static_cast<int32_t>(919296));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
transactionalmap_putifabsent_encode(const std::string& name,
                                    boost::uuids::uuid txn_id,
                                    int64_t thread_id,
                                    const serialization::pimpl::data& key,
                                    const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.putifabsent");

    msg.set_message_type(static_cast<int32_t>(919552));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
transactionalmap_replace_encode(const std::string& name,
                                boost::uuids::uuid txn_id,
                                int64_t thread_id,
                                const serialization::pimpl::data& key,
                                const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.replace");

    msg.set_message_type(static_cast<int32_t>(919808));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
transactionalmap_replaceifsame_encode(
  const std::string& name,
  boost::uuids::uuid txn_id,
  int64_t thread_id,
  const serialization::pimpl::data& key,
  const serialization::pimpl::data& old_value,
  const serialization::pimpl::data& new_value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.replaceifsame");

    msg.set_message_type(static_cast<int32_t>(920064));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(old_value);

    msg.set(new_value, true);

    return msg;
}

ClientMessage
transactionalmap_remove_encode(const std::string& name,
                               boost::uuids::uuid txn_id,
                               int64_t thread_id,
                               const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.remove");

    msg.set_message_type(static_cast<int32_t>(920320));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
transactionalmap_delete_encode(const std::string& name,
                               boost::uuids::uuid txn_id,
                               int64_t thread_id,
                               const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.delete");

    msg.set_message_type(static_cast<int32_t>(920576));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
transactionalmap_removeifsame_encode(const std::string& name,
                                     boost::uuids::uuid txn_id,
                                     int64_t thread_id,
                                     const serialization::pimpl::data& key,
                                     const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.removeifsame");

    msg.set_message_type(static_cast<int32_t>(920832));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
transactionalmap_keyset_encode(const std::string& name,
                               boost::uuids::uuid txn_id,
                               int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.keyset");

    msg.set_message_type(static_cast<int32_t>(921088));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionalmap_keysetwithpredicate_encode(
  const std::string& name,
  boost::uuids::uuid txn_id,
  int64_t thread_id,
  const serialization::pimpl::data& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.keysetwithpredicate");

    msg.set_message_type(static_cast<int32_t>(921344));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
transactionalmap_values_encode(const std::string& name,
                               boost::uuids::uuid txn_id,
                               int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.values");

    msg.set_message_type(static_cast<int32_t>(921600));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionalmap_valueswithpredicate_encode(
  const std::string& name,
  boost::uuids::uuid txn_id,
  int64_t thread_id,
  const serialization::pimpl::data& predicate)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmap.valueswithpredicate");

    msg.set_message_type(static_cast<int32_t>(921856));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(predicate, true);

    return msg;
}

ClientMessage
transactionalmultimap_put_encode(const std::string& name,
                                 boost::uuids::uuid txn_id,
                                 int64_t thread_id,
                                 const serialization::pimpl::data& key,
                                 const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmultimap.put");

    msg.set_message_type(static_cast<int32_t>(983296));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
transactionalmultimap_get_encode(const std::string& name,
                                 boost::uuids::uuid txn_id,
                                 int64_t thread_id,
                                 const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmultimap.get");

    msg.set_message_type(static_cast<int32_t>(983552));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
transactionalmultimap_remove_encode(const std::string& name,
                                    boost::uuids::uuid txn_id,
                                    int64_t thread_id,
                                    const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmultimap.remove");

    msg.set_message_type(static_cast<int32_t>(983808));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
transactionalmultimap_removeentry_encode(
  const std::string& name,
  boost::uuids::uuid txn_id,
  int64_t thread_id,
  const serialization::pimpl::data& key,
  const serialization::pimpl::data& value)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmultimap.removeentry");

    msg.set_message_type(static_cast<int32_t>(984064));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key);

    msg.set(value, true);

    return msg;
}

ClientMessage
transactionalmultimap_valuecount_encode(const std::string& name,
                                        boost::uuids::uuid txn_id,
                                        int64_t thread_id,
                                        const serialization::pimpl::data& key)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmultimap.valuecount");

    msg.set_message_type(static_cast<int32_t>(984320));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(key, true);

    return msg;
}

ClientMessage
transactionalmultimap_size_encode(const std::string& name,
                                  boost::uuids::uuid txn_id,
                                  int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalmultimap.size");

    msg.set_message_type(static_cast<int32_t>(984576));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionalset_add_encode(const std::string& name,
                            boost::uuids::uuid txn_id,
                            int64_t thread_id,
                            const serialization::pimpl::data& item)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalset.add");

    msg.set_message_type(static_cast<int32_t>(1048832));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(item, true);

    return msg;
}

ClientMessage
transactionalset_remove_encode(const std::string& name,
                               boost::uuids::uuid txn_id,
                               int64_t thread_id,
                               const serialization::pimpl::data& item)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalset.remove");

    msg.set_message_type(static_cast<int32_t>(1049088));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(item, true);

    return msg;
}

ClientMessage
transactionalset_size_encode(const std::string& name,
                             boost::uuids::uuid txn_id,
                             int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalset.size");

    msg.set_message_type(static_cast<int32_t>(1049344));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionallist_add_encode(const std::string& name,
                             boost::uuids::uuid txn_id,
                             int64_t thread_id,
                             const serialization::pimpl::data& item)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionallist.add");

    msg.set_message_type(static_cast<int32_t>(1114368));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(item, true);

    return msg;
}

ClientMessage
transactionallist_remove_encode(const std::string& name,
                                boost::uuids::uuid txn_id,
                                int64_t thread_id,
                                const serialization::pimpl::data& item)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionallist.remove");

    msg.set_message_type(static_cast<int32_t>(1114624));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name);

    msg.set(item, true);

    return msg;
}

ClientMessage
transactionallist_size_encode(const std::string& name,
                              boost::uuids::uuid txn_id,
                              int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionallist.size");

    msg.set_message_type(static_cast<int32_t>(1114880));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionalqueue_offer_encode(const std::string& name,
                                boost::uuids::uuid txn_id,
                                int64_t thread_id,
                                const serialization::pimpl::data& item,
                                int64_t timeout)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalqueue.offer");

    msg.set_message_type(static_cast<int32_t>(1179904));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(timeout);
    msg.set(name);

    msg.set(item, true);

    return msg;
}

ClientMessage
transactionalqueue_poll_encode(const std::string& name,
                               boost::uuids::uuid txn_id,
                               int64_t thread_id,
                               int64_t timeout)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE +
      ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalqueue.poll");

    msg.set_message_type(static_cast<int32_t>(1180416));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(timeout);
    msg.set(name, true);

    return msg;
}

ClientMessage
transactionalqueue_size_encode(const std::string& name,
                               boost::uuids::uuid txn_id,
                               int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("transactionalqueue.size");

    msg.set_message_type(static_cast<int32_t>(1180928));
    msg.set_partition_id(-1);

    msg.set(txn_id);
    msg.set(thread_id);
    msg.set(name, true);

    return msg;
}

ClientMessage
transaction_commit_encode(boost::uuids::uuid transaction_id, int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(false);
    msg.set_operation_name("transaction.commit");

    msg.set_message_type(static_cast<int32_t>(1376512));
    msg.set_partition_id(-1);

    msg.set(transaction_id);
    msg.set(thread_id);
    return msg;
}

ClientMessage
transaction_create_encode(int64_t timeout,
                          int32_t durability,
                          int32_t transaction_type,
                          int64_t thread_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE +
      ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(false);
    msg.set_operation_name("transaction.create");

    msg.set_message_type(static_cast<int32_t>(1376768));
    msg.set_partition_id(-1);

    msg.set(timeout);
    msg.set(durability);
    msg.set(transaction_type);
    msg.set(thread_id);
    return msg;
}

ClientMessage
transaction_rollback_encode(boost::uuids::uuid transaction_id,
                            int64_t thread_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN +
                                ClientMessage::UUID_SIZE +
                                ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size, true);
    msg.set_retryable(false);
    msg.set_operation_name("transaction.rollback");

    msg.set_message_type(static_cast<int32_t>(1377024));
    msg.set_partition_id(-1);

    msg.set(transaction_id);
    msg.set(thread_id);
    return msg;
}

ClientMessage
ringbuffer_size_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("ringbuffer.size");

    msg.set_message_type(static_cast<int32_t>(1507584));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
ringbuffer_tailsequence_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("ringbuffer.tailsequence");

    msg.set_message_type(static_cast<int32_t>(1507840));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
ringbuffer_headsequence_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("ringbuffer.headsequence");

    msg.set_message_type(static_cast<int32_t>(1508096));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
ringbuffer_capacity_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("ringbuffer.capacity");

    msg.set_message_type(static_cast<int32_t>(1508352));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
ringbuffer_remainingcapacity_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("ringbuffer.remainingcapacity");

    msg.set_message_type(static_cast<int32_t>(1508608));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
ringbuffer_add_encode(const std::string& name,
                      int32_t overflow_policy,
                      const serialization::pimpl::data& value)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("ringbuffer.add");

    msg.set_message_type(static_cast<int32_t>(1508864));
    msg.set_partition_id(-1);

    msg.set(overflow_policy);
    msg.set(name);

    msg.set(value, true);

    return msg;
}

ClientMessage
ringbuffer_readone_encode(const std::string& name, int64_t sequence)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("ringbuffer.readone");

    msg.set_message_type(static_cast<int32_t>(1509120));
    msg.set_partition_id(-1);

    msg.set(sequence);
    msg.set(name, true);

    return msg;
}

ClientMessage
ringbuffer_addall_encode(
  const std::string& name,
  const std::vector<serialization::pimpl::data>& value_list,
  int32_t overflow_policy)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("ringbuffer.addall");

    msg.set_message_type(static_cast<int32_t>(1509376));
    msg.set_partition_id(-1);

    msg.set(overflow_policy);
    msg.set(name);

    msg.set(value_list, true);

    return msg;
}

ClientMessage
ringbuffer_readmany_encode(const std::string& name,
                           int64_t start_sequence,
                           int32_t min_count,
                           int32_t max_count,
                           const serialization::pimpl::data* filter)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("ringbuffer.readmany");

    msg.set_message_type(static_cast<int32_t>(1509632));
    msg.set_partition_id(-1);

    msg.set(start_sequence);
    msg.set(min_count);
    msg.set(max_count);
    msg.set(name);

    msg.set_nullable(filter, true);

    return msg;
}

ClientMessage
flakeidgenerator_newidbatch_encode(const std::string& name, int32_t batch_size)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("flakeidgenerator.newidbatch");

    msg.set_message_type(static_cast<int32_t>(1835264));
    msg.set_partition_id(-1);

    msg.set(batch_size);
    msg.set(name, true);

    return msg;
}

ClientMessage
pncounter_get_encode(
  const std::string& name,
  const std::vector<std::pair<boost::uuids::uuid, int64_t>>& replica_timestamps,
  boost::uuids::uuid target_replica_uuid)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("pncounter.get");

    msg.set_message_type(static_cast<int32_t>(1900800));
    msg.set_partition_id(-1);

    msg.set(target_replica_uuid);
    msg.set(name);

    msg.set(replica_timestamps, true);

    return msg;
}

ClientMessage
pncounter_add_encode(
  const std::string& name,
  int64_t delta,
  bool get_before_update,
  const std::vector<std::pair<boost::uuids::uuid, int64_t>>& replica_timestamps,
  boost::uuids::uuid target_replica_uuid)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::UINT8_SIZE + ClientMessage::UUID_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("pncounter.add");

    msg.set_message_type(static_cast<int32_t>(1901056));
    msg.set_partition_id(-1);

    msg.set(delta);
    msg.set(get_before_update);
    msg.set(target_replica_uuid);
    msg.set(name);

    msg.set(replica_timestamps, true);

    return msg;
}

ClientMessage
pncounter_getconfiguredreplicacount_encode(const std::string& name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("pncounter.getconfiguredreplicacount");

    msg.set_message_type(static_cast<int32_t>(1901312));
    msg.set_partition_id(-1);

    msg.set(name, true);

    return msg;
}

ClientMessage
cpgroup_createcpgroup_encode(const std::string& proxy_name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("cpgroup.createcpgroup");

    msg.set_message_type(static_cast<int32_t>(1966336));
    msg.set_partition_id(-1);

    msg.set(proxy_name, true);

    return msg;
}

ClientMessage
cpgroup_destroycpobject_encode(const cp::raft_group_id& group_id,
                               const std::string& service_name,
                               const std::string& object_name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("cpgroup.destroycpobject");

    msg.set_message_type(static_cast<int32_t>(1966592));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(service_name);

    msg.set(object_name, true);

    return msg;
}

ClientMessage
cpsession_createsession_encode(const cp::raft_group_id& group_id,
                               const std::string& endpoint_name)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("cpsession.createsession");

    msg.set_message_type(static_cast<int32_t>(2031872));
    msg.set_partition_id(-1);

    msg.set(group_id);

    msg.set(endpoint_name, true);

    return msg;
}

ClientMessage
cpsession_closesession_encode(const cp::raft_group_id& group_id,
                              int64_t session_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("cpsession.closesession");

    msg.set_message_type(static_cast<int32_t>(2032128));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(group_id, true);

    return msg;
}

ClientMessage
cpsession_heartbeatsession_encode(const cp::raft_group_id& group_id,
                                  int64_t session_id)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("cpsession.heartbeatsession");

    msg.set_message_type(static_cast<int32_t>(2032384));
    msg.set_partition_id(-1);

    msg.set(session_id);
    msg.set(group_id, true);

    return msg;
}

ClientMessage
cpsession_generatethreadid_encode(const cp::raft_group_id& group_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("cpsession.generatethreadid");

    msg.set_message_type(static_cast<int32_t>(2032640));
    msg.set_partition_id(-1);

    msg.set(group_id, true);

    return msg;
}

ClientMessage
sql_close_encode(const sql::impl::query_id& query_id)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("sql.close");

    msg.set_message_type(static_cast<int32_t>(2163456));
    msg.set_partition_id(-1);

    msg.set(query_id, true);

    return msg;
}

ClientMessage
sql_execute_encode(const std::string& sql,
                   const std::vector<serialization::pimpl::data>& parameters,
                   int64_t timeout_millis,
                   int32_t cursor_buffer_size,
                   const std::string* schema,
                   byte expected_result_type,
                   const sql::impl::query_id& query_id,
                   bool skip_update_statistics)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT64_SIZE +
      ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE +
      ClientMessage::UINT8_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("sql.execute");

    msg.set_message_type(static_cast<int32_t>(2163712));
    msg.set_partition_id(-1);

    msg.set(timeout_millis);
    msg.set(cursor_buffer_size);
    msg.set(expected_result_type);
    msg.set(skip_update_statistics);
    msg.set(sql);

    msg.set(parameters);

    msg.set_nullable(schema);

    msg.set(query_id, true);

    return msg;
}

ClientMessage
sql_fetch_encode(const sql::impl::query_id& query_id,
                 int32_t cursor_buffer_size)
{
    size_t initial_frame_size =
      ClientMessage::REQUEST_HEADER_LEN + ClientMessage::INT32_SIZE;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(false);
    msg.set_operation_name("sql.fetch");

    msg.set_message_type(static_cast<int32_t>(2163968));
    msg.set_partition_id(-1);

    msg.set(cursor_buffer_size);
    msg.set(query_id, true);

    return msg;
}

ClientMessage
client_sendschema_encode(const serialization::pimpl::schema& schema)
{
    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN;
    ClientMessage msg(initial_frame_size);
    msg.set_retryable(true);
    msg.set_operation_name("client.sendschema");

    msg.set_message_type(static_cast<int32_t>(4864));
    msg.set_partition_id(-1);

    msg.set(schema, true);

    return msg;
}

} // namespace codec
} // namespace protocol
} // namespace client
} // namespace hazelcast
