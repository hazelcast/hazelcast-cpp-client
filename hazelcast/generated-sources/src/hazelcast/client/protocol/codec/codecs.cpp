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

#include <boost/uuid/uuid.hpp>
#include "hazelcast/client/Member.h"

#include "codecs.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                ClientMessage client_authentication_encode(const std::string  & cluster_name, const std::string  * username, const std::string  * password, boost::uuids::uuid uuid, const std::string  & client_type, byte serialization_version, const std::string  & client_hazelcast_version, const std::string  & client_name, const std::vector<std::string>  & labels) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Client.Authentication");

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

                ClientMessage client_authenticationcustom_encode(const std::string  & cluster_name, const std::vector<byte>  & credentials, boost::uuids::uuid uuid, const std::string  & client_type, byte serialization_version, const std::string  & client_hazelcast_version, const std::string  & client_name, const std::vector<std::string>  & labels) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Client.AuthenticationCustom");

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

                ClientMessage client_addclusterviewlistener_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.AddClusterViewListener");

                    msg.set_message_type(static_cast<int32_t>(768));
                    msg.set_partition_id(-1);

                    return msg;
                }

                void client_addclusterviewlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 770) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto version = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto memberInfos = msg.get<std::vector<Member>>();

                        handle_membersview(version, memberInfos);
                        return;
                    }
                    if (messageType == 771) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto version = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto partitions = msg.get<std::vector<std::pair<boost::uuids::uuid, std::vector<int>>>>();
                        handle_partitionsview(version, partitions);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[client_addclusterviewlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage client_createproxy_encode(const std::string  & name, const std::string  & service_name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.CreateProxy");

                    msg.set_message_type(static_cast<int32_t>(1024));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(service_name, true);

                    return msg;
                }

                ClientMessage client_destroyproxy_encode(const std::string  & name, const std::string  & service_name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.DestroyProxy");

                    msg.set_message_type(static_cast<int32_t>(1280));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(service_name, true);

                    return msg;
                }

                ClientMessage client_addpartitionlostlistener_encode(bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.AddPartitionLostListener");

                    msg.set_message_type(static_cast<int32_t>(1536));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    return msg;
                }

                void client_addpartitionlostlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 1538) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto partitionId = msg.get<int32_t>();
                        auto lostBackupCount = msg.get<int32_t>();
                        auto source = msg.get<boost::uuids::uuid>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        handle_partitionlost(partitionId, lostBackupCount, source);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[client_addpartitionlostlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage client_removepartitionlostlistener_encode(boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(true);
                    msg.set_operation_name("Client.RemovePartitionLostListener");

                    msg.set_message_type(static_cast<int32_t>(1792));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    return msg;
                }

                ClientMessage client_getdistributedobjects_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.GetDistributedObjects");

                    msg.set_message_type(static_cast<int32_t>(2048));
                    msg.set_partition_id(-1);

                    return msg;
                }

                ClientMessage client_adddistributedobjectlistener_encode(bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.AddDistributedObjectListener");

                    msg.set_message_type(static_cast<int32_t>(2304));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    return msg;
                }

                void client_adddistributedobjectlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 2306) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto source = msg.get<boost::uuids::uuid>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto name = msg.get<std::string>();
                        auto serviceName = msg.get<std::string>();
                        auto eventType = msg.get<std::string>();
                        handle_distributedobject(name, serviceName, eventType, source);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[client_adddistributedobjectlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage client_removedistributedobjectlistener_encode(boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(true);
                    msg.set_operation_name("Client.RemoveDistributedObjectListener");

                    msg.set_message_type(static_cast<int32_t>(2560));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    return msg;
                }

                ClientMessage client_ping_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(true);
                    msg.set_operation_name("Client.Ping");

                    msg.set_message_type(static_cast<int32_t>(2816));
                    msg.set_partition_id(-1);

                    return msg;
                }

                ClientMessage client_statistics_encode(int64_t timestamp, const std::string  & client_attributes, const std::vector<byte>  & metrics_blob) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.Statistics");

                    msg.set_message_type(static_cast<int32_t>(3072));
                    msg.set_partition_id(-1);

                    msg.set(timestamp);
                    msg.set(client_attributes);

                    msg.set(metrics_blob, true);

                    return msg;
                }

                ClientMessage client_deployclasses_encode(const std::vector<std::pair<std::string, std::vector<byte>>>  & class_definitions) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.DeployClasses");

                    msg.set_message_type(static_cast<int32_t>(3328));
                    msg.set_partition_id(-1);

                    msg.set(class_definitions, true);

                    return msg;
                }

                ClientMessage client_createproxies_encode(const std::vector<std::pair<std::string, std::string>>  & proxies) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.CreateProxies");

                    msg.set_message_type(static_cast<int32_t>(3584));
                    msg.set_partition_id(-1);

                    msg.set(proxies, true);

                    return msg;
                }

                ClientMessage client_localbackuplistener_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Client.LocalBackupListener");

                    msg.set_message_type(static_cast<int32_t>(3840));
                    msg.set_partition_id(-1);

                    return msg;
                }

                void client_localbackuplistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 3842) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto sourceInvocationCorrelationId = msg.get<int64_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        handle_backup(sourceInvocationCorrelationId);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[client_localbackuplistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage client_triggerpartitionassignment_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(true);
                    msg.set_operation_name("Client.TriggerPartitionAssignment");

                    msg.set_message_type(static_cast<int32_t>(4096));
                    msg.set_partition_id(-1);

                    return msg;
                }

                ClientMessage map_put_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Put");

                    msg.set_message_type(static_cast<int32_t>(65792));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_get_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.Get");

                    msg.set_message_type(static_cast<int32_t>(66048));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_remove_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Remove");

                    msg.set_message_type(static_cast<int32_t>(66304));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_replace_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Replace");

                    msg.set_message_type(static_cast<int32_t>(66560));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_replaceifsame_encode(const std::string  & name, const Data  & key, const Data  & test_value, const Data  & value, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.ReplaceIfSame");

                    msg.set_message_type(static_cast<int32_t>(66816));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(test_value);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_containskey_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.ContainsKey");

                    msg.set_message_type(static_cast<int32_t>(67072));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_containsvalue_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.ContainsValue");

                    msg.set_message_type(static_cast<int32_t>(67328));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_removeifsame_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.RemoveIfSame");

                    msg.set_message_type(static_cast<int32_t>(67584));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_delete_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Delete");

                    msg.set_message_type(static_cast<int32_t>(67840));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_flush_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Flush");

                    msg.set_message_type(static_cast<int32_t>(68096));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_tryremove_encode(const std::string  & name, const Data  & key, int64_t thread_id, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.TryRemove");

                    msg.set_message_type(static_cast<int32_t>(68352));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_tryput_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.TryPut");

                    msg.set_message_type(static_cast<int32_t>(68608));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_puttransient_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.PutTransient");

                    msg.set_message_type(static_cast<int32_t>(68864));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_putifabsent_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.PutIfAbsent");

                    msg.set_message_type(static_cast<int32_t>(69120));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_set_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Set");

                    msg.set_message_type(static_cast<int32_t>(69376));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_lock_encode(const std::string  & name, const Data  & key, int64_t thread_id, int64_t ttl, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.Lock");

                    msg.set_message_type(static_cast<int32_t>(69632));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(reference_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_trylock_encode(const std::string  & name, const Data  & key, int64_t thread_id, int64_t lease, int64_t timeout, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.TryLock");

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

                ClientMessage map_islocked_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.IsLocked");

                    msg.set_message_type(static_cast<int32_t>(70144));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_unlock_encode(const std::string  & name, const Data  & key, int64_t thread_id, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.Unlock");

                    msg.set_message_type(static_cast<int32_t>(70400));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(reference_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_addinterceptor_encode(const std::string  & name, const Data  & interceptor) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddInterceptor");

                    msg.set_message_type(static_cast<int32_t>(70656));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(interceptor, true);

                    return msg;
                }

                ClientMessage map_removeinterceptor_encode(const std::string  & name, const std::string  & id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.RemoveInterceptor");

                    msg.set_message_type(static_cast<int32_t>(70912));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(id, true);

                    return msg;
                }

                ClientMessage map_addentrylistenertokeywithpredicate_encode(const std::string  & name, const Data  & key, const Data  & predicate, bool include_value, int32_t listener_flags, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddEntryListenerToKeyWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(71168));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(listener_flags);
                    msg.set(local_only);
                    msg.set(name);

                    msg.set(key);

                    msg.set(predicate, true);

                    return msg;
                }

                void map_addentrylistenertokeywithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 71170) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[map_addentrylistenertokeywithpredicate_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage map_addentrylistenerwithpredicate_encode(const std::string  & name, const Data  & predicate, bool include_value, int32_t listener_flags, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddEntryListenerWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(71424));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(listener_flags);
                    msg.set(local_only);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                void map_addentrylistenerwithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 71426) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[map_addentrylistenerwithpredicate_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage map_addentrylistenertokey_encode(const std::string  & name, const Data  & key, bool include_value, int32_t listener_flags, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddEntryListenerToKey");

                    msg.set_message_type(static_cast<int32_t>(71680));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(listener_flags);
                    msg.set(local_only);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void map_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 71682) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[map_addentrylistenertokey_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage map_addentrylistener_encode(const std::string  & name, bool include_value, int32_t listener_flags, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddEntryListener");

                    msg.set_message_type(static_cast<int32_t>(71936));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(listener_flags);
                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void map_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 71938) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[map_addentrylistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage map_removeentrylistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.RemoveEntryListener");

                    msg.set_message_type(static_cast<int32_t>(72192));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_addpartitionlostlistener_encode(const std::string  & name, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddPartitionLostListener");

                    msg.set_message_type(static_cast<int32_t>(72448));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void map_addpartitionlostlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 72450) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto partitionId = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        handle_mappartitionlost(partitionId, uuid);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[map_addpartitionlostlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage map_removepartitionlostlistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.RemovePartitionLostListener");

                    msg.set_message_type(static_cast<int32_t>(72704));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_getentryview_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.GetEntryView");

                    msg.set_message_type(static_cast<int32_t>(72960));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_evict_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Evict");

                    msg.set_message_type(static_cast<int32_t>(73216));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_evictall_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.EvictAll");

                    msg.set_message_type(static_cast<int32_t>(73472));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_loadall_encode(const std::string  & name, bool replace_existing_values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.LoadAll");

                    msg.set_message_type(static_cast<int32_t>(73728));
                    msg.set_partition_id(-1);

                    msg.set(replace_existing_values);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_loadgivenkeys_encode(const std::string  & name, const std::vector<Data>  & keys, bool replace_existing_values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.LoadGivenKeys");

                    msg.set_message_type(static_cast<int32_t>(73984));
                    msg.set_partition_id(-1);

                    msg.set(replace_existing_values);
                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_keyset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.KeySet");

                    msg.set_message_type(static_cast<int32_t>(74240));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_getall_encode(const std::string  & name, const std::vector<Data>  & keys) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.GetAll");

                    msg.set_message_type(static_cast<int32_t>(74496));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_values_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.Values");

                    msg.set_message_type(static_cast<int32_t>(74752));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_entryset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.EntrySet");

                    msg.set_message_type(static_cast<int32_t>(75008));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_keysetwithpredicate_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.KeySetWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(75264));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_valueswithpredicate_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.ValuesWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(75520));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_entrieswithpredicate_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.EntriesWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(75776));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_addindex_encode(const std::string  & name, const config::index_config  & index_config) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddIndex");

                    msg.set_message_type(static_cast<int32_t>(76032));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(index_config, true);

                    return msg;
                }

                ClientMessage map_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.Size");

                    msg.set_message_type(static_cast<int32_t>(76288));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.IsEmpty");

                    msg.set_message_type(static_cast<int32_t>(76544));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_putall_encode(const std::string  & name, const std::vector<std::pair<Data, Data>>  & entries, bool trigger_map_loader) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.PutAll");

                    msg.set_message_type(static_cast<int32_t>(76800));
                    msg.set_partition_id(-1);

                    msg.set(trigger_map_loader);
                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage map_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.Clear");

                    msg.set_message_type(static_cast<int32_t>(77056));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_executeonkey_encode(const std::string  & name, const Data  & entry_processor, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.ExecuteOnKey");

                    msg.set_message_type(static_cast<int32_t>(77312));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(entry_processor);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_submittokey_encode(const std::string  & name, const Data  & entry_processor, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.SubmitToKey");

                    msg.set_message_type(static_cast<int32_t>(77568));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(entry_processor);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_executeonallkeys_encode(const std::string  & name, const Data  & entry_processor) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.ExecuteOnAllKeys");

                    msg.set_message_type(static_cast<int32_t>(77824));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(entry_processor, true);

                    return msg;
                }

                ClientMessage map_executewithpredicate_encode(const std::string  & name, const Data  & entry_processor, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.ExecuteWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(78080));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(entry_processor);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_executeonkeys_encode(const std::string  & name, const Data  & entry_processor, const std::vector<Data>  & keys) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.ExecuteOnKeys");

                    msg.set_message_type(static_cast<int32_t>(78336));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(entry_processor);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_forceunlock_encode(const std::string  & name, const Data  & key, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.ForceUnlock");

                    msg.set_message_type(static_cast<int32_t>(78592));
                    msg.set_partition_id(-1);

                    msg.set(reference_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_keysetwithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.KeySetWithPagingPredicate");

                    msg.set_message_type(static_cast<int32_t>(78848));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_valueswithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.ValuesWithPagingPredicate");

                    msg.set_message_type(static_cast<int32_t>(79104));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_entrieswithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.EntriesWithPagingPredicate");

                    msg.set_message_type(static_cast<int32_t>(79360));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_fetchkeys_encode(const std::string  & name, const std::vector<std::pair<int32_t, int32_t>>  & iteration_pointers, int32_t batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.FetchKeys");

                    msg.set_message_type(static_cast<int32_t>(79616));
                    msg.set_partition_id(-1);

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iteration_pointers, true);

                    return msg;
                }

                ClientMessage map_fetchentries_encode(const std::string  & name, const std::vector<std::pair<int32_t, int32_t>>  & iteration_pointers, int32_t batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.FetchEntries");

                    msg.set_message_type(static_cast<int32_t>(79872));
                    msg.set_partition_id(-1);

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iteration_pointers, true);

                    return msg;
                }

                ClientMessage map_aggregate_encode(const std::string  & name, const Data  & aggregator) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.Aggregate");

                    msg.set_message_type(static_cast<int32_t>(80128));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(aggregator, true);

                    return msg;
                }

                ClientMessage map_aggregatewithpredicate_encode(const std::string  & name, const Data  & aggregator, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.AggregateWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(80384));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(aggregator);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_project_encode(const std::string  & name, const Data  & projection) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.Project");

                    msg.set_message_type(static_cast<int32_t>(80640));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(projection, true);

                    return msg;
                }

                ClientMessage map_projectwithpredicate_encode(const std::string  & name, const Data  & projection, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.ProjectWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(80896));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(projection);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_fetchnearcacheinvalidationmetadata_encode(const std::vector<std::string>  & names, boost::uuids::uuid uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.FetchNearCacheInvalidationMetadata");

                    msg.set_message_type(static_cast<int32_t>(81152));
                    msg.set_partition_id(-1);

                    msg.set(uuid);
                    msg.set(names, true);

                    return msg;
                }

                ClientMessage map_removeall_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.RemoveAll");

                    msg.set_message_type(static_cast<int32_t>(81408));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_addnearcacheinvalidationlistener_encode(const std::string  & name, int32_t listener_flags, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.AddNearCacheInvalidationListener");

                    msg.set_message_type(static_cast<int32_t>(81664));
                    msg.set_partition_id(-1);

                    msg.set(listener_flags);
                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void map_addnearcacheinvalidationlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 81666) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto sourceUuid = msg.get<boost::uuids::uuid>();
                        auto partitionUuid = msg.get<boost::uuids::uuid>();
                        auto sequence = msg.get<int64_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        handle_imapinvalidation(key, sourceUuid, partitionUuid, sequence);
                        return;
                    }
                    if (messageType == 81667) {
                        msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN);

                        auto keys = msg.get<std::vector<Data>>();

                        auto sourceUuids = msg.get<std::vector<boost::uuids::uuid>>();
                        auto partitionUuids = msg.get<std::vector<boost::uuids::uuid>>();
                        auto sequences = msg.get<std::vector<int64_t>>();
                        handle_imapbatchinvalidation(keys, sourceUuids, partitionUuids, sequences);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[map_addnearcacheinvalidationlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage map_fetchwithquery_encode(const std::string  & name, const std::vector<std::pair<int32_t, int32_t>>  & iteration_pointers, int32_t batch, const Data  & projection, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.FetchWithQuery");

                    msg.set_message_type(static_cast<int32_t>(81920));
                    msg.set_partition_id(-1);

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iteration_pointers);

                    msg.set(projection);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_eventjournalsubscribe_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.EventJournalSubscribe");

                    msg.set_message_type(static_cast<int32_t>(82176));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_eventjournalread_encode(const std::string  & name, int64_t start_sequence, int32_t min_size, int32_t max_size, const Data  * predicate, const Data  * projection) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Map.EventJournalRead");

                    msg.set_message_type(static_cast<int32_t>(82432));
                    msg.set_partition_id(-1);

                    msg.set(start_sequence);
                    msg.set(min_size);
                    msg.set(max_size);
                    msg.set(name);

                    msg.set_nullable(predicate);

                    msg.set_nullable(projection, true);

                    return msg;
                }

                ClientMessage map_setttl_encode(const std::string  & name, const Data  & key, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.SetTtl");

                    msg.set_message_type(static_cast<int32_t>(82688));
                    msg.set_partition_id(-1);

                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_putwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl, int64_t max_idle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.PutWithMaxIdle");

                    msg.set_message_type(static_cast<int32_t>(82944));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(max_idle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_puttransientwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl, int64_t max_idle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.PutTransientWithMaxIdle");

                    msg.set_message_type(static_cast<int32_t>(83200));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(max_idle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_putifabsentwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl, int64_t max_idle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.PutIfAbsentWithMaxIdle");

                    msg.set_message_type(static_cast<int32_t>(83456));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(max_idle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_setwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id, int64_t ttl, int64_t max_idle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Map.SetWithMaxIdle");

                    msg.set_message_type(static_cast<int32_t>(83712));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(max_idle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_put_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.Put");

                    msg.set_message_type(static_cast<int32_t>(131328));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_get_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.Get");

                    msg.set_message_type(static_cast<int32_t>(131584));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_remove_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.Remove");

                    msg.set_message_type(static_cast<int32_t>(131840));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_keyset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.KeySet");

                    msg.set_message_type(static_cast<int32_t>(132096));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_values_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.Values");

                    msg.set_message_type(static_cast<int32_t>(132352));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_entryset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.EntrySet");

                    msg.set_message_type(static_cast<int32_t>(132608));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_containskey_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.ContainsKey");

                    msg.set_message_type(static_cast<int32_t>(132864));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_containsvalue_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.ContainsValue");

                    msg.set_message_type(static_cast<int32_t>(133120));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_containsentry_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.ContainsEntry");

                    msg.set_message_type(static_cast<int32_t>(133376));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.Size");

                    msg.set_message_type(static_cast<int32_t>(133632));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.Clear");

                    msg.set_message_type(static_cast<int32_t>(133888));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_valuecount_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.ValueCount");

                    msg.set_message_type(static_cast<int32_t>(134144));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_addentrylistenertokey_encode(const std::string  & name, const Data  & key, bool include_value, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.AddEntryListenerToKey");

                    msg.set_message_type(static_cast<int32_t>(134400));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(local_only);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void multimap_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 134402) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[multimap_addentrylistenertokey_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage multimap_addentrylistener_encode(const std::string  & name, bool include_value, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.AddEntryListener");

                    msg.set_message_type(static_cast<int32_t>(134656));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void multimap_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 134658) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[multimap_addentrylistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage multimap_removeentrylistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.RemoveEntryListener");

                    msg.set_message_type(static_cast<int32_t>(134912));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_lock_encode(const std::string  & name, const Data  & key, int64_t thread_id, int64_t ttl, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.Lock");

                    msg.set_message_type(static_cast<int32_t>(135168));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(ttl);
                    msg.set(reference_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_trylock_encode(const std::string  & name, const Data  & key, int64_t thread_id, int64_t lease, int64_t timeout, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.TryLock");

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

                ClientMessage multimap_islocked_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.IsLocked");

                    msg.set_message_type(static_cast<int32_t>(135680));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_unlock_encode(const std::string  & name, const Data  & key, int64_t thread_id, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.Unlock");

                    msg.set_message_type(static_cast<int32_t>(135936));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(reference_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_forceunlock_encode(const std::string  & name, const Data  & key, int64_t reference_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("MultiMap.ForceUnlock");

                    msg.set_message_type(static_cast<int32_t>(136192));
                    msg.set_partition_id(-1);

                    msg.set(reference_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_removeentry_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.RemoveEntry");

                    msg.set_message_type(static_cast<int32_t>(136448));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_delete_encode(const std::string  & name, const Data  & key, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.Delete");

                    msg.set_message_type(static_cast<int32_t>(136704));
                    msg.set_partition_id(-1);

                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_putall_encode(const std::string  & name, const std::vector<std::pair<Data, std::vector<Data>>>  & entries) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("MultiMap.PutAll");

                    msg.set_message_type(static_cast<int32_t>(136960));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage queue_offer_encode(const std::string  & name, const Data  & value, int64_t timeout_millis) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Offer");

                    msg.set_message_type(static_cast<int32_t>(196864));
                    msg.set_partition_id(-1);

                    msg.set(timeout_millis);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_put_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Put");

                    msg.set_message_type(static_cast<int32_t>(197120));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Size");

                    msg.set_message_type(static_cast<int32_t>(197376));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_remove_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Remove");

                    msg.set_message_type(static_cast<int32_t>(197632));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_poll_encode(const std::string  & name, int64_t timeout_millis) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Poll");

                    msg.set_message_type(static_cast<int32_t>(197888));
                    msg.set_partition_id(-1);

                    msg.set(timeout_millis);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_take_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Take");

                    msg.set_message_type(static_cast<int32_t>(198144));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_peek_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Peek");

                    msg.set_message_type(static_cast<int32_t>(198400));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_iterator_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Iterator");

                    msg.set_message_type(static_cast<int32_t>(198656));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_drainto_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.DrainTo");

                    msg.set_message_type(static_cast<int32_t>(198912));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_draintomaxsize_encode(const std::string  & name, int32_t max_size) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.DrainToMaxSize");

                    msg.set_message_type(static_cast<int32_t>(199168));
                    msg.set_partition_id(-1);

                    msg.set(max_size);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_contains_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Contains");

                    msg.set_message_type(static_cast<int32_t>(199424));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_containsall_encode(const std::string  & name, const std::vector<Data>  & data_list) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.ContainsAll");

                    msg.set_message_type(static_cast<int32_t>(199680));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(data_list, true);

                    return msg;
                }

                ClientMessage queue_compareandremoveall_encode(const std::string  & name, const std::vector<Data>  & data_list) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.CompareAndRemoveAll");

                    msg.set_message_type(static_cast<int32_t>(199936));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(data_list, true);

                    return msg;
                }

                ClientMessage queue_compareandretainall_encode(const std::string  & name, const std::vector<Data>  & data_list) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.CompareAndRetainAll");

                    msg.set_message_type(static_cast<int32_t>(200192));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(data_list, true);

                    return msg;
                }

                ClientMessage queue_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.Clear");

                    msg.set_message_type(static_cast<int32_t>(200448));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_addall_encode(const std::string  & name, const std::vector<Data>  & data_list) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.AddAll");

                    msg.set_message_type(static_cast<int32_t>(200704));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(data_list, true);

                    return msg;
                }

                ClientMessage queue_addlistener_encode(const std::string  & name, bool include_value, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.AddListener");

                    msg.set_message_type(static_cast<int32_t>(200960));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void queue_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 200962) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto eventType = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.get_nullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[queue_addlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage queue_removelistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Queue.RemoveListener");

                    msg.set_message_type(static_cast<int32_t>(201216));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_remainingcapacity_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.RemainingCapacity");

                    msg.set_message_type(static_cast<int32_t>(201472));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Queue.IsEmpty");

                    msg.set_message_type(static_cast<int32_t>(201728));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage topic_publish_encode(const std::string  & name, const Data  & message) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Topic.Publish");

                    msg.set_message_type(static_cast<int32_t>(262400));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(message, true);

                    return msg;
                }

                ClientMessage topic_addmessagelistener_encode(const std::string  & name, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Topic.AddMessageListener");

                    msg.set_message_type(static_cast<int32_t>(262656));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void topic_addmessagelistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 262658) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto publishTime = msg.get<int64_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.get<Data>();
                        handle_topic(item, publishTime, uuid);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[topic_addmessagelistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage topic_removemessagelistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Topic.RemoveMessageListener");

                    msg.set_message_type(static_cast<int32_t>(262912));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage topic_publishall_encode(const std::string  & name, const std::vector<Data>  & messages) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Topic.PublishAll");

                    msg.set_message_type(static_cast<int32_t>(263168));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(messages, true);

                    return msg;
                }

                ClientMessage list_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.Size");

                    msg.set_message_type(static_cast<int32_t>(327936));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_contains_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.Contains");

                    msg.set_message_type(static_cast<int32_t>(328192));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_containsall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.ContainsAll");

                    msg.set_message_type(static_cast<int32_t>(328448));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_add_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.Add");

                    msg.set_message_type(static_cast<int32_t>(328704));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_remove_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.Remove");

                    msg.set_message_type(static_cast<int32_t>(328960));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_addall_encode(const std::string  & name, const std::vector<Data>  & value_list) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.AddAll");

                    msg.set_message_type(static_cast<int32_t>(329216));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value_list, true);

                    return msg;
                }

                ClientMessage list_compareandremoveall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.CompareAndRemoveAll");

                    msg.set_message_type(static_cast<int32_t>(329472));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_compareandretainall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.CompareAndRetainAll");

                    msg.set_message_type(static_cast<int32_t>(329728));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.Clear");

                    msg.set_message_type(static_cast<int32_t>(329984));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_getall_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.GetAll");

                    msg.set_message_type(static_cast<int32_t>(330240));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_addlistener_encode(const std::string  & name, bool include_value, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.AddListener");

                    msg.set_message_type(static_cast<int32_t>(330496));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void list_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 330498) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto eventType = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.get_nullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[list_addlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage list_removelistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.RemoveListener");

                    msg.set_message_type(static_cast<int32_t>(330752));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.IsEmpty");

                    msg.set_message_type(static_cast<int32_t>(331008));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_addallwithindex_encode(const std::string  & name, int32_t index, const std::vector<Data>  & value_list) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.AddAllWithIndex");

                    msg.set_message_type(static_cast<int32_t>(331264));
                    msg.set_partition_id(-1);

                    msg.set(index);
                    msg.set(name);

                    msg.set(value_list, true);

                    return msg;
                }

                ClientMessage list_get_encode(const std::string  & name, int32_t index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.Get");

                    msg.set_message_type(static_cast<int32_t>(331520));
                    msg.set_partition_id(-1);

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_set_encode(const std::string  & name, int32_t index, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.Set");

                    msg.set_message_type(static_cast<int32_t>(331776));
                    msg.set_partition_id(-1);

                    msg.set(index);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_addwithindex_encode(const std::string  & name, int32_t index, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.AddWithIndex");

                    msg.set_message_type(static_cast<int32_t>(332032));
                    msg.set_partition_id(-1);

                    msg.set(index);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_removewithindex_encode(const std::string  & name, int32_t index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("List.RemoveWithIndex");

                    msg.set_message_type(static_cast<int32_t>(332288));
                    msg.set_partition_id(-1);

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_lastindexof_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.LastIndexOf");

                    msg.set_message_type(static_cast<int32_t>(332544));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_indexof_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.IndexOf");

                    msg.set_message_type(static_cast<int32_t>(332800));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_sub_encode(const std::string  & name, int32_t from, int32_t to) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.Sub");

                    msg.set_message_type(static_cast<int32_t>(333056));
                    msg.set_partition_id(-1);

                    msg.set(from);
                    msg.set(to);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_iterator_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.Iterator");

                    msg.set_message_type(static_cast<int32_t>(333312));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_listiterator_encode(const std::string  & name, int32_t index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("List.ListIterator");

                    msg.set_message_type(static_cast<int32_t>(333568));
                    msg.set_partition_id(-1);

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.Size");

                    msg.set_message_type(static_cast<int32_t>(393472));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_contains_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.Contains");

                    msg.set_message_type(static_cast<int32_t>(393728));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_containsall_encode(const std::string  & name, const std::vector<Data>  & items) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.ContainsAll");

                    msg.set_message_type(static_cast<int32_t>(393984));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(items, true);

                    return msg;
                }

                ClientMessage set_add_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.Add");

                    msg.set_message_type(static_cast<int32_t>(394240));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_remove_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.Remove");

                    msg.set_message_type(static_cast<int32_t>(394496));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_addall_encode(const std::string  & name, const std::vector<Data>  & value_list) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.AddAll");

                    msg.set_message_type(static_cast<int32_t>(394752));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value_list, true);

                    return msg;
                }

                ClientMessage set_compareandremoveall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.CompareAndRemoveAll");

                    msg.set_message_type(static_cast<int32_t>(395008));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage set_compareandretainall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.CompareAndRetainAll");

                    msg.set_message_type(static_cast<int32_t>(395264));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage set_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.Clear");

                    msg.set_message_type(static_cast<int32_t>(395520));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_getall_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.GetAll");

                    msg.set_message_type(static_cast<int32_t>(395776));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_addlistener_encode(const std::string  & name, bool include_value, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.AddListener");

                    msg.set_message_type(static_cast<int32_t>(396032));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void set_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 396034) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto eventType = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.get_nullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[set_addlistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage set_removelistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Set.RemoveListener");

                    msg.set_message_type(static_cast<int32_t>(396288));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Set.IsEmpty");

                    msg.set_message_type(static_cast<int32_t>(396544));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage fencedlock_lock_encode(const raft_group_id  & group_id, const std::string  & name, int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("FencedLock.Lock");

                    msg.set_message_type(static_cast<int32_t>(459008));
                    msg.set_partition_id(-1);

                    msg.set(session_id);
                    msg.set(thread_id);
                    msg.set(invocation_uid);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage fencedlock_trylock_encode(const raft_group_id  & group_id, const std::string  & name, int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid, int64_t timeout_ms) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("FencedLock.TryLock");

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

                ClientMessage fencedlock_unlock_encode(const raft_group_id  & group_id, const std::string  & name, int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("FencedLock.Unlock");

                    msg.set_message_type(static_cast<int32_t>(459520));
                    msg.set_partition_id(-1);

                    msg.set(session_id);
                    msg.set(thread_id);
                    msg.set(invocation_uid);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage fencedlock_getlockownership_encode(const raft_group_id  & group_id, const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("FencedLock.GetLockOwnership");

                    msg.set_message_type(static_cast<int32_t>(459776));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage executorservice_shutdown_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ExecutorService.Shutdown");

                    msg.set_message_type(static_cast<int32_t>(524544));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage executorservice_isshutdown_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ExecutorService.IsShutdown");

                    msg.set_message_type(static_cast<int32_t>(524800));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage executorservice_cancelonpartition_encode(boost::uuids::uuid uuid, bool interrupt) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("ExecutorService.CancelOnPartition");

                    msg.set_message_type(static_cast<int32_t>(525056));
                    msg.set_partition_id(-1);

                    msg.set(uuid);
                    msg.set(interrupt);
                    return msg;
                }

                ClientMessage executorservice_cancelonmember_encode(boost::uuids::uuid uuid, boost::uuids::uuid member_uuid, bool interrupt) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("ExecutorService.CancelOnMember");

                    msg.set_message_type(static_cast<int32_t>(525312));
                    msg.set_partition_id(-1);

                    msg.set(uuid);
                    msg.set(member_uuid);
                    msg.set(interrupt);
                    return msg;
                }

                ClientMessage executorservice_submittopartition_encode(const std::string  & name, boost::uuids::uuid uuid, const Data  & callable) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ExecutorService.SubmitToPartition");

                    msg.set_message_type(static_cast<int32_t>(525568));
                    msg.set_partition_id(-1);

                    msg.set(uuid);
                    msg.set(name);

                    msg.set(callable, true);

                    return msg;
                }

                ClientMessage executorservice_submittomember_encode(const std::string  & name, boost::uuids::uuid uuid, const Data  & callable, boost::uuids::uuid member_uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ExecutorService.SubmitToMember");

                    msg.set_message_type(static_cast<int32_t>(525824));
                    msg.set_partition_id(-1);

                    msg.set(uuid);
                    msg.set(member_uuid);
                    msg.set(name);

                    msg.set(callable, true);

                    return msg;
                }

                ClientMessage atomiclong_apply_encode(const raft_group_id  & group_id, const std::string  & name, const Data  & function) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicLong.Apply");

                    msg.set_message_type(static_cast<int32_t>(590080));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name);

                    msg.set(function, true);

                    return msg;
                }

                ClientMessage atomiclong_alter_encode(const raft_group_id  & group_id, const std::string  & name, const Data  & function, int32_t return_value_type) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicLong.Alter");

                    msg.set_message_type(static_cast<int32_t>(590336));
                    msg.set_partition_id(-1);

                    msg.set(return_value_type);
                    msg.set(group_id);

                    msg.set(name);

                    msg.set(function, true);

                    return msg;
                }

                ClientMessage atomiclong_addandget_encode(const raft_group_id  & group_id, const std::string  & name, int64_t delta) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicLong.AddAndGet");

                    msg.set_message_type(static_cast<int32_t>(590592));
                    msg.set_partition_id(-1);

                    msg.set(delta);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage atomiclong_compareandset_encode(const raft_group_id  & group_id, const std::string  & name, int64_t expected, int64_t updated) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicLong.CompareAndSet");

                    msg.set_message_type(static_cast<int32_t>(590848));
                    msg.set_partition_id(-1);

                    msg.set(expected);
                    msg.set(updated);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage atomiclong_get_encode(const raft_group_id  & group_id, const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("AtomicLong.Get");

                    msg.set_message_type(static_cast<int32_t>(591104));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage atomiclong_getandadd_encode(const raft_group_id  & group_id, const std::string  & name, int64_t delta) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicLong.GetAndAdd");

                    msg.set_message_type(static_cast<int32_t>(591360));
                    msg.set_partition_id(-1);

                    msg.set(delta);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage atomiclong_getandset_encode(const raft_group_id  & group_id, const std::string  & name, int64_t new_value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicLong.GetAndSet");

                    msg.set_message_type(static_cast<int32_t>(591616));
                    msg.set_partition_id(-1);

                    msg.set(new_value);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage atomicref_apply_encode(const raft_group_id  & group_id, const std::string  & name, const Data  & function, int32_t return_value_type, bool alter) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicRef.Apply");

                    msg.set_message_type(static_cast<int32_t>(655616));
                    msg.set_partition_id(-1);

                    msg.set(return_value_type);
                    msg.set(alter);
                    msg.set(group_id);

                    msg.set(name);

                    msg.set(function, true);

                    return msg;
                }

                ClientMessage atomicref_compareandset_encode(const raft_group_id  & group_id, const std::string  & name, const Data  * old_value, const Data  * new_value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicRef.CompareAndSet");

                    msg.set_message_type(static_cast<int32_t>(655872));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name);

                    msg.set_nullable(old_value);

                    msg.set_nullable(new_value, true);

                    return msg;
                }

                ClientMessage atomicref_contains_encode(const raft_group_id  & group_id, const std::string  & name, const Data  * value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("AtomicRef.Contains");

                    msg.set_message_type(static_cast<int32_t>(656128));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name);

                    msg.set_nullable(value, true);

                    return msg;
                }

                ClientMessage atomicref_get_encode(const raft_group_id  & group_id, const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("AtomicRef.Get");

                    msg.set_message_type(static_cast<int32_t>(656384));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage atomicref_set_encode(const raft_group_id  & group_id, const std::string  & name, const Data  * new_value, bool return_old_value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("AtomicRef.Set");

                    msg.set_message_type(static_cast<int32_t>(656640));
                    msg.set_partition_id(-1);

                    msg.set(return_old_value);
                    msg.set(group_id);

                    msg.set(name);

                    msg.set_nullable(new_value, true);

                    return msg;
                }

                ClientMessage countdownlatch_trysetcount_encode(const raft_group_id  & group_id, const std::string  & name, int32_t count) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CountDownLatch.TrySetCount");

                    msg.set_message_type(static_cast<int32_t>(721152));
                    msg.set_partition_id(-1);

                    msg.set(count);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage countdownlatch_await_encode(const raft_group_id  & group_id, const std::string  & name, boost::uuids::uuid invocation_uid, int64_t timeout_ms) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CountDownLatch.Await");

                    msg.set_message_type(static_cast<int32_t>(721408));
                    msg.set_partition_id(-1);

                    msg.set(invocation_uid);
                    msg.set(timeout_ms);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage countdownlatch_countdown_encode(const raft_group_id  & group_id, const std::string  & name, boost::uuids::uuid invocation_uid, int32_t expected_round) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CountDownLatch.CountDown");

                    msg.set_message_type(static_cast<int32_t>(721664));
                    msg.set_partition_id(-1);

                    msg.set(invocation_uid);
                    msg.set(expected_round);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage countdownlatch_getcount_encode(const raft_group_id  & group_id, const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CountDownLatch.GetCount");

                    msg.set_message_type(static_cast<int32_t>(721920));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage countdownlatch_getround_encode(const raft_group_id  & group_id, const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CountDownLatch.GetRound");

                    msg.set_message_type(static_cast<int32_t>(722176));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage semaphore_init_encode(const raft_group_id  & group_id, const std::string  & name, int32_t permits) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Semaphore.Init");

                    msg.set_message_type(static_cast<int32_t>(786688));
                    msg.set_partition_id(-1);

                    msg.set(permits);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage semaphore_acquire_encode(const raft_group_id  & group_id, const std::string  & name, int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid, int32_t permits, int64_t timeout_ms) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Semaphore.Acquire");

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

                ClientMessage semaphore_release_encode(const raft_group_id  & group_id, const std::string  & name, int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid, int32_t permits) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Semaphore.Release");

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

                ClientMessage semaphore_drain_encode(const raft_group_id  & group_id, const std::string  & name, int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Semaphore.Drain");

                    msg.set_message_type(static_cast<int32_t>(787456));
                    msg.set_partition_id(-1);

                    msg.set(session_id);
                    msg.set(thread_id);
                    msg.set(invocation_uid);
                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage semaphore_change_encode(const raft_group_id  & group_id, const std::string  & name, int64_t session_id, int64_t thread_id, boost::uuids::uuid invocation_uid, int32_t permits) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::UUID_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Semaphore.Change");

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

                ClientMessage semaphore_availablepermits_encode(const raft_group_id  & group_id, const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Semaphore.AvailablePermits");

                    msg.set_message_type(static_cast<int32_t>(787968));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage semaphore_getsemaphoretype_encode(const std::string  & proxy_name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Semaphore.GetSemaphoreType");

                    msg.set_message_type(static_cast<int32_t>(788224));
                    msg.set_partition_id(-1);

                    msg.set(proxy_name, true);

                    return msg;
                }

                ClientMessage replicatedmap_put_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.Put");

                    msg.set_message_type(static_cast<int32_t>(852224));
                    msg.set_partition_id(-1);

                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage replicatedmap_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.Size");

                    msg.set_message_type(static_cast<int32_t>(852480));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.IsEmpty");

                    msg.set_message_type(static_cast<int32_t>(852736));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_containskey_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.ContainsKey");

                    msg.set_message_type(static_cast<int32_t>(852992));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage replicatedmap_containsvalue_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.ContainsValue");

                    msg.set_message_type(static_cast<int32_t>(853248));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage replicatedmap_get_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.Get");

                    msg.set_message_type(static_cast<int32_t>(853504));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage replicatedmap_remove_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.Remove");

                    msg.set_message_type(static_cast<int32_t>(853760));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage replicatedmap_putall_encode(const std::string  & name, const std::vector<std::pair<Data, Data>>  & entries) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.PutAll");

                    msg.set_message_type(static_cast<int32_t>(854016));
                    msg.set_partition_id(-1);

                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage replicatedmap_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.Clear");

                    msg.set_message_type(static_cast<int32_t>(854272));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_addentrylistenertokeywithpredicate_encode(const std::string  & name, const Data  & key, const Data  & predicate, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.AddEntryListenerToKeyWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(854528));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    msg.set(name);

                    msg.set(key);

                    msg.set(predicate, true);

                    return msg;
                }

                void replicatedmap_addentrylistenertokeywithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 854530) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[replicatedmap_addentrylistenertokeywithpredicate_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage replicatedmap_addentrylistenerwithpredicate_encode(const std::string  & name, const Data  & predicate, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.AddEntryListenerWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(854784));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                void replicatedmap_addentrylistenerwithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 854786) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[replicatedmap_addentrylistenerwithpredicate_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage replicatedmap_addentrylistenertokey_encode(const std::string  & name, const Data  & key, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.AddEntryListenerToKey");

                    msg.set_message_type(static_cast<int32_t>(855040));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void replicatedmap_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 855042) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[replicatedmap_addentrylistenertokey_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage replicatedmap_addentrylistener_encode(const std::string  & name, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.AddEntryListener");

                    msg.set_message_type(static_cast<int32_t>(855296));
                    msg.set_partition_id(-1);

                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void replicatedmap_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 855298) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[replicatedmap_addentrylistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage replicatedmap_removeentrylistener_encode(const std::string  & name, boost::uuids::uuid registration_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.RemoveEntryListener");

                    msg.set_message_type(static_cast<int32_t>(855552));
                    msg.set_partition_id(-1);

                    msg.set(registration_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_keyset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.KeySet");

                    msg.set_message_type(static_cast<int32_t>(855808));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_values_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.Values");

                    msg.set_message_type(static_cast<int32_t>(856064));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_entryset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("ReplicatedMap.EntrySet");

                    msg.set_message_type(static_cast<int32_t>(856320));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_addnearcacheentrylistener_encode(const std::string  & name, bool include_value, bool local_only) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("ReplicatedMap.AddNearCacheEntryListener");

                    msg.set_message_type(static_cast<int32_t>(856576));
                    msg.set_partition_id(-1);

                    msg.set(include_value);
                    msg.set(local_only);
                    msg.set(name, true);

                    return msg;
                }

                void replicatedmap_addnearcacheentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.get_message_type();
                    if (messageType == 856578) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.get_nullable<Data>();
                        auto value = msg.get_nullable<Data>();
                        auto oldValue = msg.get_nullable<Data>();
                        auto mergingValue = msg.get_nullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    HZ_LOG(*get_logger(), warning,
                        boost::str(boost::format("[replicatedmap_addnearcacheentrylistener_handler::handle] "
                                                 "Unknown message type (%1%) received on event handler.")
                                                 % messageType)
                    );
                }

                ClientMessage transactionalmap_containskey_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.ContainsKey");

                    msg.set_message_type(static_cast<int32_t>(917760));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_get_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Get");

                    msg.set_message_type(static_cast<int32_t>(918016));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_getforupdate_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.GetForUpdate");

                    msg.set_message_type(static_cast<int32_t>(918272));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_size_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Size");

                    msg.set_message_type(static_cast<int32_t>(918528));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_isempty_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.IsEmpty");

                    msg.set_message_type(static_cast<int32_t>(918784));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_put_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & value, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Put");

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

                ClientMessage transactionalmap_set_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Set");

                    msg.set_message_type(static_cast<int32_t>(919296));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_putifabsent_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.PutIfAbsent");

                    msg.set_message_type(static_cast<int32_t>(919552));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_replace_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Replace");

                    msg.set_message_type(static_cast<int32_t>(919808));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_replaceifsame_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & old_value, const Data  & new_value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.ReplaceIfSame");

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

                ClientMessage transactionalmap_remove_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Remove");

                    msg.set_message_type(static_cast<int32_t>(920320));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_delete_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Delete");

                    msg.set_message_type(static_cast<int32_t>(920576));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_removeifsame_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.RemoveIfSame");

                    msg.set_message_type(static_cast<int32_t>(920832));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_keyset_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.KeySet");

                    msg.set_message_type(static_cast<int32_t>(921088));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_keysetwithpredicate_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.KeySetWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(921344));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage transactionalmap_values_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.Values");

                    msg.set_message_type(static_cast<int32_t>(921600));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_valueswithpredicate_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.ValuesWithPredicate");

                    msg.set_message_type(static_cast<int32_t>(921856));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage transactionalmap_containsvalue_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMap.ContainsValue");

                    msg.set_message_type(static_cast<int32_t>(922112));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_put_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMultiMap.Put");

                    msg.set_message_type(static_cast<int32_t>(983296));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_get_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMultiMap.Get");

                    msg.set_message_type(static_cast<int32_t>(983552));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_remove_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMultiMap.Remove");

                    msg.set_message_type(static_cast<int32_t>(983808));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_removeentry_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMultiMap.RemoveEntry");

                    msg.set_message_type(static_cast<int32_t>(984064));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_valuecount_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMultiMap.ValueCount");

                    msg.set_message_type(static_cast<int32_t>(984320));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_size_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalMultiMap.Size");

                    msg.set_message_type(static_cast<int32_t>(984576));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalset_add_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalSet.Add");

                    msg.set_message_type(static_cast<int32_t>(1048832));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalset_remove_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalSet.Remove");

                    msg.set_message_type(static_cast<int32_t>(1049088));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalset_size_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalSet.Size");

                    msg.set_message_type(static_cast<int32_t>(1049344));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionallist_add_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalList.Add");

                    msg.set_message_type(static_cast<int32_t>(1114368));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionallist_remove_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalList.Remove");

                    msg.set_message_type(static_cast<int32_t>(1114624));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionallist_size_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalList.Size");

                    msg.set_message_type(static_cast<int32_t>(1114880));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_offer_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, const Data  & item, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalQueue.Offer");

                    msg.set_message_type(static_cast<int32_t>(1179904));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalqueue_take_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalQueue.Take");

                    msg.set_message_type(static_cast<int32_t>(1180160));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_poll_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalQueue.Poll");

                    msg.set_message_type(static_cast<int32_t>(1180416));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(timeout);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_peek_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalQueue.Peek");

                    msg.set_message_type(static_cast<int32_t>(1180672));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(timeout);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_size_encode(const std::string  & name, boost::uuids::uuid txn_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("TransactionalQueue.Size");

                    msg.set_message_type(static_cast<int32_t>(1180928));
                    msg.set_partition_id(-1);

                    msg.set(txn_id);
                    msg.set(thread_id);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transaction_commit_encode(boost::uuids::uuid transaction_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Transaction.Commit");

                    msg.set_message_type(static_cast<int32_t>(1376512));
                    msg.set_partition_id(-1);

                    msg.set(transaction_id);
                    msg.set(thread_id);
                    return msg;
                }

                ClientMessage transaction_create_encode(int64_t timeout, int32_t durability, int32_t transaction_type, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Transaction.Create");

                    msg.set_message_type(static_cast<int32_t>(1376768));
                    msg.set_partition_id(-1);

                    msg.set(timeout);
                    msg.set(durability);
                    msg.set(transaction_type);
                    msg.set(thread_id);
                    return msg;
                }

                ClientMessage transaction_rollback_encode(boost::uuids::uuid transaction_id, int64_t thread_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.set_retryable(false);
                    msg.set_operation_name("Transaction.Rollback");

                    msg.set_message_type(static_cast<int32_t>(1377024));
                    msg.set_partition_id(-1);

                    msg.set(transaction_id);
                    msg.set(thread_id);
                    return msg;
                }

                ClientMessage ringbuffer_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Ringbuffer.Size");

                    msg.set_message_type(static_cast<int32_t>(1507584));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_tailsequence_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Ringbuffer.TailSequence");

                    msg.set_message_type(static_cast<int32_t>(1507840));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_headsequence_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Ringbuffer.HeadSequence");

                    msg.set_message_type(static_cast<int32_t>(1508096));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_capacity_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Ringbuffer.Capacity");

                    msg.set_message_type(static_cast<int32_t>(1508352));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_remainingcapacity_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Ringbuffer.RemainingCapacity");

                    msg.set_message_type(static_cast<int32_t>(1508608));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_add_encode(const std::string  & name, int32_t overflow_policy, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Ringbuffer.Add");

                    msg.set_message_type(static_cast<int32_t>(1508864));
                    msg.set_partition_id(-1);

                    msg.set(overflow_policy);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage ringbuffer_readone_encode(const std::string  & name, int64_t sequence) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Ringbuffer.ReadOne");

                    msg.set_message_type(static_cast<int32_t>(1509120));
                    msg.set_partition_id(-1);

                    msg.set(sequence);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_addall_encode(const std::string  & name, const std::vector<Data>  & value_list, int32_t overflow_policy) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("Ringbuffer.AddAll");

                    msg.set_message_type(static_cast<int32_t>(1509376));
                    msg.set_partition_id(-1);

                    msg.set(overflow_policy);
                    msg.set(name);

                    msg.set(value_list, true);

                    return msg;
                }

                ClientMessage ringbuffer_readmany_encode(const std::string  & name, int64_t start_sequence, int32_t min_count, int32_t max_count, const Data  * filter) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("Ringbuffer.ReadMany");

                    msg.set_message_type(static_cast<int32_t>(1509632));
                    msg.set_partition_id(-1);

                    msg.set(start_sequence);
                    msg.set(min_count);
                    msg.set(max_count);
                    msg.set(name);

                    msg.set_nullable(filter, true);

                    return msg;
                }

                ClientMessage flakeidgenerator_newidbatch_encode(const std::string  & name, int32_t batch_size) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("FlakeIdGenerator.NewIdBatch");

                    msg.set_message_type(static_cast<int32_t>(1835264));
                    msg.set_partition_id(-1);

                    msg.set(batch_size);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage pncounter_get_encode(const std::string  & name, const std::vector<std::pair<boost::uuids::uuid, int64_t>>  & replica_timestamps, boost::uuids::uuid target_replica_uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("PNCounter.Get");

                    msg.set_message_type(static_cast<int32_t>(1900800));
                    msg.set_partition_id(-1);

                    msg.set(target_replica_uuid);
                    msg.set(name);

                    msg.set(replica_timestamps, true);

                    return msg;
                }

                ClientMessage pncounter_add_encode(const std::string  & name, int64_t delta, bool get_before_update, const std::vector<std::pair<boost::uuids::uuid, int64_t>>  & replica_timestamps, boost::uuids::uuid target_replica_uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::UINT8_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(false);
                    msg.set_operation_name("PNCounter.Add");

                    msg.set_message_type(static_cast<int32_t>(1901056));
                    msg.set_partition_id(-1);

                    msg.set(delta);
                    msg.set(get_before_update);
                    msg.set(target_replica_uuid);
                    msg.set(name);

                    msg.set(replica_timestamps, true);

                    return msg;
                }

                ClientMessage pncounter_getconfiguredreplicacount_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("PNCounter.GetConfiguredReplicaCount");

                    msg.set_message_type(static_cast<int32_t>(1901312));
                    msg.set_partition_id(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage cpgroup_createcpgroup_encode(const std::string  & proxy_name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CPGroup.CreateCPGroup");

                    msg.set_message_type(static_cast<int32_t>(1966336));
                    msg.set_partition_id(-1);

                    msg.set(proxy_name, true);

                    return msg;
                }

                ClientMessage cpgroup_destroycpobject_encode(const raft_group_id  & group_id, const std::string  & service_name, const std::string  & object_name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CPGroup.DestroyCPObject");

                    msg.set_message_type(static_cast<int32_t>(1966592));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(service_name);

                    msg.set(object_name, true);

                    return msg;
                }

                ClientMessage cpsession_createsession_encode(const raft_group_id  & group_id, const std::string  & endpoint_name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CPSession.CreateSession");

                    msg.set_message_type(static_cast<int32_t>(2031872));
                    msg.set_partition_id(-1);

                    msg.set(group_id);

                    msg.set(endpoint_name, true);

                    return msg;
                }

                ClientMessage cpsession_closesession_encode(const raft_group_id  & group_id, int64_t session_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CPSession.CloseSession");

                    msg.set_message_type(static_cast<int32_t>(2032128));
                    msg.set_partition_id(-1);

                    msg.set(session_id);
                    msg.set(group_id, true);

                    return msg;
                }

                ClientMessage cpsession_heartbeatsession_encode(const raft_group_id  & group_id, int64_t session_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CPSession.HeartbeatSession");

                    msg.set_message_type(static_cast<int32_t>(2032384));
                    msg.set_partition_id(-1);

                    msg.set(session_id);
                    msg.set(group_id, true);

                    return msg;
                }

                ClientMessage cpsession_generatethreadid_encode(const raft_group_id  & group_id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.set_retryable(true);
                    msg.set_operation_name("CPSession.GenerateThreadId");

                    msg.set_message_type(static_cast<int32_t>(2032640));
                    msg.set_partition_id(-1);

                    msg.set(group_id, true);

                    return msg;
                }

            }
        }
    }
}

