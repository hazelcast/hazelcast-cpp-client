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
                ClientMessage client_authentication_encode(const std::string  & clusterName, const std::string  * username, const std::string  * password, boost::uuids::uuid uuid, const std::string  & clientType, byte serializationVersion, const std::string  & clientHazelcastVersion, const std::string  & clientName, const std::vector<std::string>  & labels) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.Authentication");

                    msg.setMessageType(static_cast<int32_t>(256));
                    msg.setPartitionId(-1);

                    msg.set(uuid);
                    msg.set(serializationVersion);
                    msg.set(clusterName);

                    msg.setNullable(username);

                    msg.setNullable(password);

                    msg.set(clientType);

                    msg.set(clientHazelcastVersion);

                    msg.set(clientName);

                    msg.set(labels, true);

                    return msg;
                }

                ClientMessage client_authenticationcustom_encode(const std::string  & clusterName, const std::vector<byte>  & credentials, boost::uuids::uuid uuid, const std::string  & clientType, byte serializationVersion, const std::string  & clientHazelcastVersion, const std::string  & clientName, const std::vector<std::string>  & labels) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.AuthenticationCustom");

                    msg.setMessageType(static_cast<int32_t>(512));
                    msg.setPartitionId(-1);

                    msg.set(uuid);
                    msg.set(serializationVersion);
                    msg.set(clusterName);

                    msg.set(credentials);

                    msg.set(clientType);

                    msg.set(clientHazelcastVersion);

                    msg.set(clientName);

                    msg.set(labels, true);

                    return msg;
                }

                ClientMessage client_addclusterviewlistener_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.AddClusterViewListener");

                    msg.setMessageType(static_cast<int32_t>(768));
                    msg.setPartitionId(-1);

                    return msg;
                }

                void client_addclusterviewlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
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
                    getLogger()->warning(
                          "[client_addclusterviewlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_createproxy_encode(const std::string  & name, const std::string  & serviceName) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.CreateProxy");

                    msg.setMessageType(static_cast<int32_t>(1024));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(serviceName, true);

                    return msg;
                }

                ClientMessage client_destroyproxy_encode(const std::string  & name, const std::string  & serviceName) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.DestroyProxy");

                    msg.setMessageType(static_cast<int32_t>(1280));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(serviceName, true);

                    return msg;
                }

                ClientMessage client_addpartitionlostlistener_encode(bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.AddPartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(1536));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    return msg;
                }

                void client_addpartitionlostlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 1538) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto partitionId = msg.get<int32_t>();
                        auto lostBackupCount = msg.get<int32_t>();
                        auto source = msg.get<boost::uuids::uuid>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        handle_partitionlost(partitionId, lostBackupCount, source);
                        return;
                    }
                    getLogger()->warning(
                          "[client_addpartitionlostlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_removepartitionlostlistener_encode(boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.RemovePartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(1792));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    return msg;
                }

                ClientMessage client_getdistributedobjects_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.GetDistributedObjects");

                    msg.setMessageType(static_cast<int32_t>(2048));
                    msg.setPartitionId(-1);

                    return msg;
                }

                ClientMessage client_adddistributedobjectlistener_encode(bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.AddDistributedObjectListener");

                    msg.setMessageType(static_cast<int32_t>(2304));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    return msg;
                }

                void client_adddistributedobjectlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
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
                    getLogger()->warning(
                          "[client_adddistributedobjectlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_removedistributedobjectlistener_encode(boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.RemoveDistributedObjectListener");

                    msg.setMessageType(static_cast<int32_t>(2560));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    return msg;
                }

                ClientMessage client_ping_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.Ping");

                    msg.setMessageType(static_cast<int32_t>(2816));
                    msg.setPartitionId(-1);

                    return msg;
                }

                ClientMessage client_statistics_encode(int64_t timestamp, const std::string  & clientAttributes, const std::vector<byte>  & metricsBlob) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.Statistics");

                    msg.setMessageType(static_cast<int32_t>(3072));
                    msg.setPartitionId(-1);

                    msg.set(timestamp);
                    msg.set(clientAttributes);

                    msg.set(metricsBlob, true);

                    return msg;
                }

                ClientMessage client_deployclasses_encode(const std::vector<std::pair<std::string, std::vector<byte>>>  & classDefinitions) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.DeployClasses");

                    msg.setMessageType(static_cast<int32_t>(3328));
                    msg.setPartitionId(-1);

                    msg.set(classDefinitions, true);

                    return msg;
                }

                ClientMessage client_createproxies_encode(const std::vector<std::pair<std::string, std::string>>  & proxies) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.CreateProxies");

                    msg.setMessageType(static_cast<int32_t>(3584));
                    msg.setPartitionId(-1);

                    msg.set(proxies, true);

                    return msg;
                }

                ClientMessage client_localbackuplistener_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Client.LocalBackupListener");

                    msg.setMessageType(static_cast<int32_t>(3840));
                    msg.setPartitionId(-1);

                    return msg;
                }

                void client_localbackuplistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 3842) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto sourceInvocationCorrelationId = msg.get<int64_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        handle_backup(sourceInvocationCorrelationId);
                        return;
                    }
                    getLogger()->warning(
                          "[client_localbackuplistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage client_triggerpartitionassignment_encode() {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(true);
                    msg.setOperationName("Client.TriggerPartitionAssignment");

                    msg.setMessageType(static_cast<int32_t>(4096));
                    msg.setPartitionId(-1);

                    return msg;
                }

                ClientMessage map_put_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Put");

                    msg.setMessageType(static_cast<int32_t>(65792));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_get_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Get");

                    msg.setMessageType(static_cast<int32_t>(66048));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_remove_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Remove");

                    msg.setMessageType(static_cast<int32_t>(66304));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_replace_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Replace");

                    msg.setMessageType(static_cast<int32_t>(66560));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_replaceifsame_encode(const std::string  & name, const Data  & key, const Data  & testValue, const Data  & value, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ReplaceIfSame");

                    msg.setMessageType(static_cast<int32_t>(66816));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(testValue);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_containskey_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(67072));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_containsvalue_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ContainsValue");

                    msg.setMessageType(static_cast<int32_t>(67328));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_removeifsame_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.RemoveIfSame");

                    msg.setMessageType(static_cast<int32_t>(67584));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_delete_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Delete");

                    msg.setMessageType(static_cast<int32_t>(67840));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_flush_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Flush");

                    msg.setMessageType(static_cast<int32_t>(68096));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_tryremove_encode(const std::string  & name, const Data  & key, int64_t threadId, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.TryRemove");

                    msg.setMessageType(static_cast<int32_t>(68352));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_tryput_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.TryPut");

                    msg.setMessageType(static_cast<int32_t>(68608));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_puttransient_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutTransient");

                    msg.setMessageType(static_cast<int32_t>(68864));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_putifabsent_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutIfAbsent");

                    msg.setMessageType(static_cast<int32_t>(69120));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_set_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Set");

                    msg.setMessageType(static_cast<int32_t>(69376));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_lock_encode(const std::string  & name, const Data  & key, int64_t threadId, int64_t ttl, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Lock");

                    msg.setMessageType(static_cast<int32_t>(69632));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_trylock_encode(const std::string  & name, const Data  & key, int64_t threadId, int64_t lease, int64_t timeout, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.TryLock");

                    msg.setMessageType(static_cast<int32_t>(69888));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(lease);
                    msg.set(timeout);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_islocked_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.IsLocked");

                    msg.setMessageType(static_cast<int32_t>(70144));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_unlock_encode(const std::string  & name, const Data  & key, int64_t threadId, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Unlock");

                    msg.setMessageType(static_cast<int32_t>(70400));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_addinterceptor_encode(const std::string  & name, const Data  & interceptor) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddInterceptor");

                    msg.setMessageType(static_cast<int32_t>(70656));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(interceptor, true);

                    return msg;
                }

                ClientMessage map_removeinterceptor_encode(const std::string  & name, const std::string  & id) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.RemoveInterceptor");

                    msg.setMessageType(static_cast<int32_t>(70912));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(id, true);

                    return msg;
                }

                ClientMessage map_addentrylistenertokeywithpredicate_encode(const std::string  & name, const Data  & key, const Data  & predicate, bool includeValue, int32_t listenerFlags, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListenerToKeyWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(71168));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key);

                    msg.set(predicate, true);

                    return msg;
                }

                void map_addentrylistenertokeywithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71170) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistenertokeywithpredicate_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_addentrylistenerwithpredicate_encode(const std::string  & name, const Data  & predicate, bool includeValue, int32_t listenerFlags, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListenerWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(71424));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                void map_addentrylistenerwithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71426) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistenerwithpredicate_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_addentrylistenertokey_encode(const std::string  & name, const Data  & key, bool includeValue, int32_t listenerFlags, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListenerToKey");

                    msg.setMessageType(static_cast<int32_t>(71680));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void map_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71682) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistenertokey_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_addentrylistener_encode(const std::string  & name, bool includeValue, int32_t listenerFlags, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddEntryListener");

                    msg.setMessageType(static_cast<int32_t>(71936));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void map_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 71938) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addentrylistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_removeentrylistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.RemoveEntryListener");

                    msg.setMessageType(static_cast<int32_t>(72192));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_addpartitionlostlistener_encode(const std::string  & name, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddPartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(72448));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void map_addpartitionlostlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 72450) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto partitionId = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        handle_mappartitionlost(partitionId, uuid);
                        return;
                    }
                    getLogger()->warning(
                          "[map_addpartitionlostlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_removepartitionlostlistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.RemovePartitionLostListener");

                    msg.setMessageType(static_cast<int32_t>(72704));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_getentryview_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.GetEntryView");

                    msg.setMessageType(static_cast<int32_t>(72960));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_evict_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Evict");

                    msg.setMessageType(static_cast<int32_t>(73216));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_evictall_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.EvictAll");

                    msg.setMessageType(static_cast<int32_t>(73472));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_loadall_encode(const std::string  & name, bool replaceExistingValues) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.LoadAll");

                    msg.setMessageType(static_cast<int32_t>(73728));
                    msg.setPartitionId(-1);

                    msg.set(replaceExistingValues);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_loadgivenkeys_encode(const std::string  & name, const std::vector<Data>  & keys, bool replaceExistingValues) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.LoadGivenKeys");

                    msg.setMessageType(static_cast<int32_t>(73984));
                    msg.setPartitionId(-1);

                    msg.set(replaceExistingValues);
                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_keyset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.KeySet");

                    msg.setMessageType(static_cast<int32_t>(74240));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_getall_encode(const std::string  & name, const std::vector<Data>  & keys) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.GetAll");

                    msg.setMessageType(static_cast<int32_t>(74496));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_values_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Values");

                    msg.setMessageType(static_cast<int32_t>(74752));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_entryset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EntrySet");

                    msg.setMessageType(static_cast<int32_t>(75008));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_keysetwithpredicate_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.KeySetWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(75264));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_valueswithpredicate_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ValuesWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(75520));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_entrieswithpredicate_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EntriesWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(75776));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_addindex_encode(const std::string  & name, const config::index_config  & indexConfig) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddIndex");

                    msg.setMessageType(static_cast<int32_t>(76032));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(indexConfig, true);

                    return msg;
                }

                ClientMessage map_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Size");

                    msg.setMessageType(static_cast<int32_t>(76288));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(76544));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_putall_encode(const std::string  & name, const std::vector<std::pair<Data, Data>>  & entries, bool triggerMapLoader) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutAll");

                    msg.setMessageType(static_cast<int32_t>(76800));
                    msg.setPartitionId(-1);

                    msg.set(triggerMapLoader);
                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage map_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.Clear");

                    msg.setMessageType(static_cast<int32_t>(77056));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_executeonkey_encode(const std::string  & name, const Data  & entryProcessor, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteOnKey");

                    msg.setMessageType(static_cast<int32_t>(77312));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_submittokey_encode(const std::string  & name, const Data  & entryProcessor, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.SubmitToKey");

                    msg.setMessageType(static_cast<int32_t>(77568));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_executeonallkeys_encode(const std::string  & name, const Data  & entryProcessor) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteOnAllKeys");

                    msg.setMessageType(static_cast<int32_t>(77824));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(entryProcessor, true);

                    return msg;
                }

                ClientMessage map_executewithpredicate_encode(const std::string  & name, const Data  & entryProcessor, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(78080));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_executeonkeys_encode(const std::string  & name, const Data  & entryProcessor, const std::vector<Data>  & keys) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.ExecuteOnKeys");

                    msg.setMessageType(static_cast<int32_t>(78336));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(entryProcessor);

                    msg.set(keys, true);

                    return msg;
                }

                ClientMessage map_forceunlock_encode(const std::string  & name, const Data  & key, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ForceUnlock");

                    msg.setMessageType(static_cast<int32_t>(78592));
                    msg.setPartitionId(-1);

                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_keysetwithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.KeySetWithPagingPredicate");

                    msg.setMessageType(static_cast<int32_t>(78848));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_valueswithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ValuesWithPagingPredicate");

                    msg.setMessageType(static_cast<int32_t>(79104));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_entrieswithpagingpredicate_encode(const std::string  & name, const codec::holder::paging_predicate_holder  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EntriesWithPagingPredicate");

                    msg.setMessageType(static_cast<int32_t>(79360));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_fetchkeys_encode(const std::string  & name, const std::vector<std::pair<int32_t, int32_t>>  & iterationPointers, int32_t batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.FetchKeys");

                    msg.setMessageType(static_cast<int32_t>(79616));
                    msg.setPartitionId(-1);

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers, true);

                    return msg;
                }

                ClientMessage map_fetchentries_encode(const std::string  & name, const std::vector<std::pair<int32_t, int32_t>>  & iterationPointers, int32_t batch) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.FetchEntries");

                    msg.setMessageType(static_cast<int32_t>(79872));
                    msg.setPartitionId(-1);

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers, true);

                    return msg;
                }

                ClientMessage map_aggregate_encode(const std::string  & name, const Data  & aggregator) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Aggregate");

                    msg.setMessageType(static_cast<int32_t>(80128));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(aggregator, true);

                    return msg;
                }

                ClientMessage map_aggregatewithpredicate_encode(const std::string  & name, const Data  & aggregator, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.AggregateWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(80384));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(aggregator);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_project_encode(const std::string  & name, const Data  & projection) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.Project");

                    msg.setMessageType(static_cast<int32_t>(80640));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(projection, true);

                    return msg;
                }

                ClientMessage map_projectwithpredicate_encode(const std::string  & name, const Data  & projection, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.ProjectWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(80896));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(projection);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_fetchnearcacheinvalidationmetadata_encode(const std::vector<std::string>  & names, boost::uuids::uuid uuid) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.FetchNearCacheInvalidationMetadata");

                    msg.setMessageType(static_cast<int32_t>(81152));
                    msg.setPartitionId(-1);

                    msg.set(uuid);
                    msg.set(names, true);

                    return msg;
                }

                ClientMessage map_removeall_encode(const std::string  & name, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.RemoveAll");

                    msg.setMessageType(static_cast<int32_t>(81408));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_addnearcacheinvalidationlistener_encode(const std::string  & name, int32_t listenerFlags, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.AddNearCacheInvalidationListener");

                    msg.setMessageType(static_cast<int32_t>(81664));
                    msg.setPartitionId(-1);

                    msg.set(listenerFlags);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void map_addnearcacheinvalidationlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 81666) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto sourceUuid = msg.get<boost::uuids::uuid>();
                        auto partitionUuid = msg.get<boost::uuids::uuid>();
                        auto sequence = msg.get<int64_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
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
                    getLogger()->warning(
                          "[map_addnearcacheinvalidationlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage map_fetchwithquery_encode(const std::string  & name, const std::vector<std::pair<int32_t, int32_t>>  & iterationPointers, int32_t batch, const Data  & projection, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.FetchWithQuery");

                    msg.setMessageType(static_cast<int32_t>(81920));
                    msg.setPartitionId(-1);

                    msg.set(batch);
                    msg.set(name);

                    msg.set(iterationPointers);

                    msg.set(projection);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage map_eventjournalsubscribe_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EventJournalSubscribe");

                    msg.setMessageType(static_cast<int32_t>(82176));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage map_eventjournalread_encode(const std::string  & name, int64_t startSequence, int32_t minSize, int32_t maxSize, const Data  * predicate, const Data  * projection) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Map.EventJournalRead");

                    msg.setMessageType(static_cast<int32_t>(82432));
                    msg.setPartitionId(-1);

                    msg.set(startSequence);
                    msg.set(minSize);
                    msg.set(maxSize);
                    msg.set(name);

                    msg.setNullable(predicate);

                    msg.setNullable(projection, true);

                    return msg;
                }

                ClientMessage map_setttl_encode(const std::string  & name, const Data  & key, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.SetTtl");

                    msg.setMessageType(static_cast<int32_t>(82688));
                    msg.setPartitionId(-1);

                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage map_putwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl, int64_t maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(82944));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_puttransientwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl, int64_t maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutTransientWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(83200));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_putifabsentwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl, int64_t maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.PutIfAbsentWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(83456));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage map_setwithmaxidle_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId, int64_t ttl, int64_t maxIdle) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Map.SetWithMaxIdle");

                    msg.setMessageType(static_cast<int32_t>(83712));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(maxIdle);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_put_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Put");

                    msg.setMessageType(static_cast<int32_t>(131328));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_get_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Get");

                    msg.setMessageType(static_cast<int32_t>(131584));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_remove_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Remove");

                    msg.setMessageType(static_cast<int32_t>(131840));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_keyset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.KeySet");

                    msg.setMessageType(static_cast<int32_t>(132096));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_values_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Values");

                    msg.setMessageType(static_cast<int32_t>(132352));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_entryset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.EntrySet");

                    msg.setMessageType(static_cast<int32_t>(132608));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_containskey_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(132864));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_containsvalue_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ContainsValue");

                    msg.setMessageType(static_cast<int32_t>(133120));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_containsentry_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ContainsEntry");

                    msg.setMessageType(static_cast<int32_t>(133376));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Size");

                    msg.setMessageType(static_cast<int32_t>(133632));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Clear");

                    msg.setMessageType(static_cast<int32_t>(133888));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_valuecount_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ValueCount");

                    msg.setMessageType(static_cast<int32_t>(134144));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_addentrylistenertokey_encode(const std::string  & name, const Data  & key, bool includeValue, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.AddEntryListenerToKey");

                    msg.setMessageType(static_cast<int32_t>(134400));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void multimap_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 134402) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[multimap_addentrylistenertokey_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage multimap_addentrylistener_encode(const std::string  & name, bool includeValue, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.AddEntryListener");

                    msg.setMessageType(static_cast<int32_t>(134656));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void multimap_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 134658) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[multimap_addentrylistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage multimap_removeentrylistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.RemoveEntryListener");

                    msg.setMessageType(static_cast<int32_t>(134912));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage multimap_lock_encode(const std::string  & name, const Data  & key, int64_t threadId, int64_t ttl, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Lock");

                    msg.setMessageType(static_cast<int32_t>(135168));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_trylock_encode(const std::string  & name, const Data  & key, int64_t threadId, int64_t lease, int64_t timeout, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.TryLock");

                    msg.setMessageType(static_cast<int32_t>(135424));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(lease);
                    msg.set(timeout);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_islocked_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.IsLocked");

                    msg.setMessageType(static_cast<int32_t>(135680));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_unlock_encode(const std::string  & name, const Data  & key, int64_t threadId, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.Unlock");

                    msg.setMessageType(static_cast<int32_t>(135936));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_forceunlock_encode(const std::string  & name, const Data  & key, int64_t referenceId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("MultiMap.ForceUnlock");

                    msg.setMessageType(static_cast<int32_t>(136192));
                    msg.setPartitionId(-1);

                    msg.set(referenceId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_removeentry_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.RemoveEntry");

                    msg.setMessageType(static_cast<int32_t>(136448));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage multimap_delete_encode(const std::string  & name, const Data  & key, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.Delete");

                    msg.setMessageType(static_cast<int32_t>(136704));
                    msg.setPartitionId(-1);

                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage multimap_putall_encode(const std::string  & name, const std::vector<std::pair<Data, std::vector<Data>>>  & entries) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("MultiMap.PutAll");

                    msg.setMessageType(static_cast<int32_t>(136960));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage queue_offer_encode(const std::string  & name, const Data  & value, int64_t timeoutMillis) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Offer");

                    msg.setMessageType(static_cast<int32_t>(196864));
                    msg.setPartitionId(-1);

                    msg.set(timeoutMillis);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_put_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Put");

                    msg.setMessageType(static_cast<int32_t>(197120));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Size");

                    msg.setMessageType(static_cast<int32_t>(197376));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_remove_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Remove");

                    msg.setMessageType(static_cast<int32_t>(197632));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_poll_encode(const std::string  & name, int64_t timeoutMillis) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Poll");

                    msg.setMessageType(static_cast<int32_t>(197888));
                    msg.setPartitionId(-1);

                    msg.set(timeoutMillis);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_take_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Take");

                    msg.setMessageType(static_cast<int32_t>(198144));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_peek_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Peek");

                    msg.setMessageType(static_cast<int32_t>(198400));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_iterator_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Iterator");

                    msg.setMessageType(static_cast<int32_t>(198656));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_drainto_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.DrainTo");

                    msg.setMessageType(static_cast<int32_t>(198912));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_draintomaxsize_encode(const std::string  & name, int32_t maxSize) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.DrainToMaxSize");

                    msg.setMessageType(static_cast<int32_t>(199168));
                    msg.setPartitionId(-1);

                    msg.set(maxSize);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_contains_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Contains");

                    msg.setMessageType(static_cast<int32_t>(199424));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage queue_containsall_encode(const std::string  & name, const std::vector<Data>  & dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.ContainsAll");

                    msg.setMessageType(static_cast<int32_t>(199680));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_compareandremoveall_encode(const std::string  & name, const std::vector<Data>  & dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.CompareAndRemoveAll");

                    msg.setMessageType(static_cast<int32_t>(199936));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_compareandretainall_encode(const std::string  & name, const std::vector<Data>  & dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.CompareAndRetainAll");

                    msg.setMessageType(static_cast<int32_t>(200192));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.Clear");

                    msg.setMessageType(static_cast<int32_t>(200448));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_addall_encode(const std::string  & name, const std::vector<Data>  & dataList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.AddAll");

                    msg.setMessageType(static_cast<int32_t>(200704));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(dataList, true);

                    return msg;
                }

                ClientMessage queue_addlistener_encode(const std::string  & name, bool includeValue, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.AddListener");

                    msg.setMessageType(static_cast<int32_t>(200960));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void queue_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 200962) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto eventType = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.getNullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    getLogger()->warning(
                          "[queue_addlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage queue_removelistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Queue.RemoveListener");

                    msg.setMessageType(static_cast<int32_t>(201216));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_remainingcapacity_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.RemainingCapacity");

                    msg.setMessageType(static_cast<int32_t>(201472));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage queue_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Queue.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(201728));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage topic_publish_encode(const std::string  & name, const Data  & message) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Topic.Publish");

                    msg.setMessageType(static_cast<int32_t>(262400));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(message, true);

                    return msg;
                }

                ClientMessage topic_addmessagelistener_encode(const std::string  & name, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Topic.AddMessageListener");

                    msg.setMessageType(static_cast<int32_t>(262656));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void topic_addmessagelistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 262658) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto publishTime = msg.get<int64_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.get<Data>();
                        handle_topic(item, publishTime, uuid);
                        return;
                    }
                    getLogger()->warning(
                          "[topic_addmessagelistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage topic_removemessagelistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Topic.RemoveMessageListener");

                    msg.setMessageType(static_cast<int32_t>(262912));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Size");

                    msg.setMessageType(static_cast<int32_t>(327936));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_contains_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Contains");

                    msg.setMessageType(static_cast<int32_t>(328192));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_containsall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.ContainsAll");

                    msg.setMessageType(static_cast<int32_t>(328448));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_add_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Add");

                    msg.setMessageType(static_cast<int32_t>(328704));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_remove_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Remove");

                    msg.setMessageType(static_cast<int32_t>(328960));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_addall_encode(const std::string  & name, const std::vector<Data>  & valueList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddAll");

                    msg.setMessageType(static_cast<int32_t>(329216));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage list_compareandremoveall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.CompareAndRemoveAll");

                    msg.setMessageType(static_cast<int32_t>(329472));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_compareandretainall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.CompareAndRetainAll");

                    msg.setMessageType(static_cast<int32_t>(329728));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage list_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Clear");

                    msg.setMessageType(static_cast<int32_t>(329984));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_getall_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.GetAll");

                    msg.setMessageType(static_cast<int32_t>(330240));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_addlistener_encode(const std::string  & name, bool includeValue, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddListener");

                    msg.setMessageType(static_cast<int32_t>(330496));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void list_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 330498) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto eventType = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.getNullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    getLogger()->warning(
                          "[list_addlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage list_removelistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.RemoveListener");

                    msg.setMessageType(static_cast<int32_t>(330752));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(331008));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_addallwithindex_encode(const std::string  & name, int32_t index, const std::vector<Data>  & valueList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddAllWithIndex");

                    msg.setMessageType(static_cast<int32_t>(331264));
                    msg.setPartitionId(-1);

                    msg.set(index);
                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage list_get_encode(const std::string  & name, int32_t index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Get");

                    msg.setMessageType(static_cast<int32_t>(331520));
                    msg.setPartitionId(-1);

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_set_encode(const std::string  & name, int32_t index, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.Set");

                    msg.setMessageType(static_cast<int32_t>(331776));
                    msg.setPartitionId(-1);

                    msg.set(index);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_addwithindex_encode(const std::string  & name, int32_t index, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.AddWithIndex");

                    msg.setMessageType(static_cast<int32_t>(332032));
                    msg.setPartitionId(-1);

                    msg.set(index);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_removewithindex_encode(const std::string  & name, int32_t index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("List.RemoveWithIndex");

                    msg.setMessageType(static_cast<int32_t>(332288));
                    msg.setPartitionId(-1);

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_lastindexof_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.LastIndexOf");

                    msg.setMessageType(static_cast<int32_t>(332544));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_indexof_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.IndexOf");

                    msg.setMessageType(static_cast<int32_t>(332800));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage list_sub_encode(const std::string  & name, int32_t from, int32_t to) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Sub");

                    msg.setMessageType(static_cast<int32_t>(333056));
                    msg.setPartitionId(-1);

                    msg.set(from);
                    msg.set(to);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_iterator_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.Iterator");

                    msg.setMessageType(static_cast<int32_t>(333312));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage list_listiterator_encode(const std::string  & name, int32_t index) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("List.ListIterator");

                    msg.setMessageType(static_cast<int32_t>(333568));
                    msg.setPartitionId(-1);

                    msg.set(index);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Size");

                    msg.setMessageType(static_cast<int32_t>(393472));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_contains_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Contains");

                    msg.setMessageType(static_cast<int32_t>(393728));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_containsall_encode(const std::string  & name, const std::vector<Data>  & items) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.ContainsAll");

                    msg.setMessageType(static_cast<int32_t>(393984));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(items, true);

                    return msg;
                }

                ClientMessage set_add_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Add");

                    msg.setMessageType(static_cast<int32_t>(394240));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_remove_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Remove");

                    msg.setMessageType(static_cast<int32_t>(394496));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage set_addall_encode(const std::string  & name, const std::vector<Data>  & valueList) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.AddAll");

                    msg.setMessageType(static_cast<int32_t>(394752));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage set_compareandremoveall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.CompareAndRemoveAll");

                    msg.setMessageType(static_cast<int32_t>(395008));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage set_compareandretainall_encode(const std::string  & name, const std::vector<Data>  & values) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.CompareAndRetainAll");

                    msg.setMessageType(static_cast<int32_t>(395264));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(values, true);

                    return msg;
                }

                ClientMessage set_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.Clear");

                    msg.setMessageType(static_cast<int32_t>(395520));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_getall_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.GetAll");

                    msg.setMessageType(static_cast<int32_t>(395776));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_addlistener_encode(const std::string  & name, bool includeValue, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.AddListener");

                    msg.setMessageType(static_cast<int32_t>(396032));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void set_addlistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 396034) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto eventType = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto item = msg.getNullable<Data>();
                        handle_item(item, uuid, eventType);
                        return;
                    }
                    getLogger()->warning(
                          "[set_addlistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage set_removelistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Set.RemoveListener");

                    msg.setMessageType(static_cast<int32_t>(396288));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage set_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Set.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(396544));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage executorservice_shutdown_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ExecutorService.Shutdown");

                    msg.setMessageType(static_cast<int32_t>(524544));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage executorservice_isshutdown_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ExecutorService.IsShutdown");

                    msg.setMessageType(static_cast<int32_t>(524800));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage executorservice_cancelonpartition_encode(boost::uuids::uuid uuid, bool interrupt) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("ExecutorService.CancelOnPartition");

                    msg.setMessageType(static_cast<int32_t>(525056));
                    msg.setPartitionId(-1);

                    msg.set(uuid);
                    msg.set(interrupt);
                    return msg;
                }

                ClientMessage executorservice_cancelonmember_encode(boost::uuids::uuid uuid, boost::uuids::uuid memberUUID, bool interrupt) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UUID_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("ExecutorService.CancelOnMember");

                    msg.setMessageType(static_cast<int32_t>(525312));
                    msg.setPartitionId(-1);

                    msg.set(uuid);
                    msg.set(memberUUID);
                    msg.set(interrupt);
                    return msg;
                }

                ClientMessage executorservice_submittopartition_encode(const std::string  & name, boost::uuids::uuid uuid, const Data  & callable) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ExecutorService.SubmitToPartition");

                    msg.setMessageType(static_cast<int32_t>(525568));
                    msg.setPartitionId(-1);

                    msg.set(uuid);
                    msg.set(name);

                    msg.set(callable, true);

                    return msg;
                }

                ClientMessage executorservice_submittomember_encode(const std::string  & name, boost::uuids::uuid uuid, const Data  & callable, boost::uuids::uuid memberUUID) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ExecutorService.SubmitToMember");

                    msg.setMessageType(static_cast<int32_t>(525824));
                    msg.setPartitionId(-1);

                    msg.set(uuid);
                    msg.set(memberUUID);
                    msg.set(name);

                    msg.set(callable, true);

                    return msg;
                }

                ClientMessage replicatedmap_put_encode(const std::string  & name, const Data  & key, const Data  & value, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.Put");

                    msg.setMessageType(static_cast<int32_t>(852224));
                    msg.setPartitionId(-1);

                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage replicatedmap_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.Size");

                    msg.setMessageType(static_cast<int32_t>(852480));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_isempty_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(852736));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_containskey_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(852992));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage replicatedmap_containsvalue_encode(const std::string  & name, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.ContainsValue");

                    msg.setMessageType(static_cast<int32_t>(853248));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage replicatedmap_get_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.Get");

                    msg.setMessageType(static_cast<int32_t>(853504));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage replicatedmap_remove_encode(const std::string  & name, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.Remove");

                    msg.setMessageType(static_cast<int32_t>(853760));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage replicatedmap_putall_encode(const std::string  & name, const std::vector<std::pair<Data, Data>>  & entries) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.PutAll");

                    msg.setMessageType(static_cast<int32_t>(854016));
                    msg.setPartitionId(-1);

                    msg.set(name);

                    msg.set(entries, true);

                    return msg;
                }

                ClientMessage replicatedmap_clear_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.Clear");

                    msg.setMessageType(static_cast<int32_t>(854272));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_addentrylistenertokeywithpredicate_encode(const std::string  & name, const Data  & key, const Data  & predicate, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.AddEntryListenerToKeyWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(854528));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key);

                    msg.set(predicate, true);

                    return msg;
                }

                void replicatedmap_addentrylistenertokeywithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 854530) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[replicatedmap_addentrylistenertokeywithpredicate_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage replicatedmap_addentrylistenerwithpredicate_encode(const std::string  & name, const Data  & predicate, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.AddEntryListenerWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(854784));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                void replicatedmap_addentrylistenerwithpredicate_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 854786) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[replicatedmap_addentrylistenerwithpredicate_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage replicatedmap_addentrylistenertokey_encode(const std::string  & name, const Data  & key, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.AddEntryListenerToKey");

                    msg.setMessageType(static_cast<int32_t>(855040));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                void replicatedmap_addentrylistenertokey_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 855042) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[replicatedmap_addentrylistenertokey_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage replicatedmap_addentrylistener_encode(const std::string  & name, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.AddEntryListener");

                    msg.setMessageType(static_cast<int32_t>(855296));
                    msg.setPartitionId(-1);

                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void replicatedmap_addentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 855298) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[replicatedmap_addentrylistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage replicatedmap_removeentrylistener_encode(const std::string  & name, boost::uuids::uuid registrationId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.RemoveEntryListener");

                    msg.setMessageType(static_cast<int32_t>(855552));
                    msg.setPartitionId(-1);

                    msg.set(registrationId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_keyset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.KeySet");

                    msg.setMessageType(static_cast<int32_t>(855808));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_values_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.Values");

                    msg.setMessageType(static_cast<int32_t>(856064));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_entryset_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("ReplicatedMap.EntrySet");

                    msg.setMessageType(static_cast<int32_t>(856320));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage replicatedmap_addnearcacheentrylistener_encode(const std::string  & name, bool includeValue, bool localOnly) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UINT8_SIZE + ClientMessage::UINT8_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("ReplicatedMap.AddNearCacheEntryListener");

                    msg.setMessageType(static_cast<int32_t>(856576));
                    msg.setPartitionId(-1);

                    msg.set(includeValue);
                    msg.set(localOnly);
                    msg.set(name, true);

                    return msg;
                }

                void replicatedmap_addnearcacheentrylistener_handler::handle(ClientMessage &msg) {
                    auto messageType = msg.getMessageType();
                    if (messageType == 856578) {
                        auto *initial_frame = reinterpret_cast<ClientMessage::frame_header_t *>(msg.rd_ptr(ClientMessage::EVENT_HEADER_LEN));
                        auto eventType = msg.get<int32_t>();
                        auto uuid = msg.get<boost::uuids::uuid>();
                        auto numberOfAffectedEntries = msg.get<int32_t>();
                        msg.seek(static_cast<int32_t>(initial_frame->frame_len));

                        auto key = msg.getNullable<Data>();
                        auto value = msg.getNullable<Data>();
                        auto oldValue = msg.getNullable<Data>();
                        auto mergingValue = msg.getNullable<Data>();
                        handle_entry(key, value, oldValue, mergingValue, eventType, uuid, numberOfAffectedEntries);
                        return;
                    }
                    getLogger()->warning(
                          "[replicatedmap_addnearcacheentrylistener_handler::handle] Unknown message type (",
                          messageType, ") received on event handler.");
                }

                ClientMessage transactionalmap_containskey_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ContainsKey");

                    msg.setMessageType(static_cast<int32_t>(917760));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_get_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Get");

                    msg.setMessageType(static_cast<int32_t>(918016));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_getforupdate_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.GetForUpdate");

                    msg.setMessageType(static_cast<int32_t>(918272));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_size_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Size");

                    msg.setMessageType(static_cast<int32_t>(918528));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_isempty_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.IsEmpty");

                    msg.setMessageType(static_cast<int32_t>(918784));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_put_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & value, int64_t ttl) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Put");

                    msg.setMessageType(static_cast<int32_t>(919040));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(ttl);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_set_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Set");

                    msg.setMessageType(static_cast<int32_t>(919296));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_putifabsent_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.PutIfAbsent");

                    msg.setMessageType(static_cast<int32_t>(919552));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_replace_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Replace");

                    msg.setMessageType(static_cast<int32_t>(919808));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_replaceifsame_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & oldValue, const Data  & newValue) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ReplaceIfSame");

                    msg.setMessageType(static_cast<int32_t>(920064));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(oldValue);

                    msg.set(newValue, true);

                    return msg;
                }

                ClientMessage transactionalmap_remove_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Remove");

                    msg.setMessageType(static_cast<int32_t>(920320));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_delete_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Delete");

                    msg.setMessageType(static_cast<int32_t>(920576));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmap_removeifsame_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.RemoveIfSame");

                    msg.setMessageType(static_cast<int32_t>(920832));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmap_keyset_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.KeySet");

                    msg.setMessageType(static_cast<int32_t>(921088));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_keysetwithpredicate_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.KeySetWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(921344));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage transactionalmap_values_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.Values");

                    msg.setMessageType(static_cast<int32_t>(921600));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalmap_valueswithpredicate_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & predicate) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ValuesWithPredicate");

                    msg.setMessageType(static_cast<int32_t>(921856));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(predicate, true);

                    return msg;
                }

                ClientMessage transactionalmap_containsvalue_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMap.ContainsValue");

                    msg.setMessageType(static_cast<int32_t>(922112));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_put_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Put");

                    msg.setMessageType(static_cast<int32_t>(983296));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_get_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Get");

                    msg.setMessageType(static_cast<int32_t>(983552));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_remove_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Remove");

                    msg.setMessageType(static_cast<int32_t>(983808));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_removeentry_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.RemoveEntry");

                    msg.setMessageType(static_cast<int32_t>(984064));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_valuecount_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & key) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.ValueCount");

                    msg.setMessageType(static_cast<int32_t>(984320));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(key, true);

                    return msg;
                }

                ClientMessage transactionalmultimap_size_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalMultiMap.Size");

                    msg.setMessageType(static_cast<int32_t>(984576));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalset_add_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalSet.Add");

                    msg.setMessageType(static_cast<int32_t>(1048832));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalset_remove_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalSet.Remove");

                    msg.setMessageType(static_cast<int32_t>(1049088));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalset_size_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalSet.Size");

                    msg.setMessageType(static_cast<int32_t>(1049344));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionallist_add_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalList.Add");

                    msg.setMessageType(static_cast<int32_t>(1114368));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionallist_remove_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & item) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalList.Remove");

                    msg.setMessageType(static_cast<int32_t>(1114624));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionallist_size_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalList.Size");

                    msg.setMessageType(static_cast<int32_t>(1114880));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_offer_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, const Data  & item, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Offer");

                    msg.setMessageType(static_cast<int32_t>(1179904));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name);

                    msg.set(item, true);

                    return msg;
                }

                ClientMessage transactionalqueue_take_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Take");

                    msg.setMessageType(static_cast<int32_t>(1180160));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_poll_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Poll");

                    msg.setMessageType(static_cast<int32_t>(1180416));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_peek_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId, int64_t timeout) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Peek");

                    msg.setMessageType(static_cast<int32_t>(1180672));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(timeout);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transactionalqueue_size_encode(const std::string  & name, boost::uuids::uuid txnId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("TransactionalQueue.Size");

                    msg.setMessageType(static_cast<int32_t>(1180928));
                    msg.setPartitionId(-1);

                    msg.set(txnId);
                    msg.set(threadId);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage transaction_commit_encode(boost::uuids::uuid transactionId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Transaction.Commit");

                    msg.setMessageType(static_cast<int32_t>(1376512));
                    msg.setPartitionId(-1);

                    msg.set(transactionId);
                    msg.set(threadId);
                    return msg;
                }

                ClientMessage transaction_create_encode(int64_t timeout, int32_t durability, int32_t transactionType, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Transaction.Create");

                    msg.setMessageType(static_cast<int32_t>(1376768));
                    msg.setPartitionId(-1);

                    msg.set(timeout);
                    msg.set(durability);
                    msg.set(transactionType);
                    msg.set(threadId);
                    return msg;
                }

                ClientMessage transaction_rollback_encode(boost::uuids::uuid transactionId, int64_t threadId) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size, true);
                    msg.setRetryable(false);
                    msg.setOperationName("Transaction.Rollback");

                    msg.setMessageType(static_cast<int32_t>(1377024));
                    msg.setPartitionId(-1);

                    msg.set(transactionId);
                    msg.set(threadId);
                    return msg;
                }

                ClientMessage ringbuffer_size_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.Size");

                    msg.setMessageType(static_cast<int32_t>(1507584));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_tailsequence_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.TailSequence");

                    msg.setMessageType(static_cast<int32_t>(1507840));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_headsequence_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.HeadSequence");

                    msg.setMessageType(static_cast<int32_t>(1508096));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_capacity_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.Capacity");

                    msg.setMessageType(static_cast<int32_t>(1508352));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_remainingcapacity_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.RemainingCapacity");

                    msg.setMessageType(static_cast<int32_t>(1508608));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_add_encode(const std::string  & name, int32_t overflowPolicy, const Data  & value) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Ringbuffer.Add");

                    msg.setMessageType(static_cast<int32_t>(1508864));
                    msg.setPartitionId(-1);

                    msg.set(overflowPolicy);
                    msg.set(name);

                    msg.set(value, true);

                    return msg;
                }

                ClientMessage ringbuffer_readone_encode(const std::string  & name, int64_t sequence) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.ReadOne");

                    msg.setMessageType(static_cast<int32_t>(1509120));
                    msg.setPartitionId(-1);

                    msg.set(sequence);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage ringbuffer_addall_encode(const std::string  & name, const std::vector<Data>  & valueList, int32_t overflowPolicy) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("Ringbuffer.AddAll");

                    msg.setMessageType(static_cast<int32_t>(1509376));
                    msg.setPartitionId(-1);

                    msg.set(overflowPolicy);
                    msg.set(name);

                    msg.set(valueList, true);

                    return msg;
                }

                ClientMessage ringbuffer_readmany_encode(const std::string  & name, int64_t startSequence, int32_t minCount, int32_t maxCount, const Data  * filter) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::INT32_SIZE + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("Ringbuffer.ReadMany");

                    msg.setMessageType(static_cast<int32_t>(1509632));
                    msg.setPartitionId(-1);

                    msg.set(startSequence);
                    msg.set(minCount);
                    msg.set(maxCount);
                    msg.set(name);

                    msg.setNullable(filter, true);

                    return msg;
                }

                ClientMessage flakeidgenerator_newidbatch_encode(const std::string  & name, int32_t batchSize) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT32_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("FlakeIdGenerator.NewIdBatch");

                    msg.setMessageType(static_cast<int32_t>(1835264));
                    msg.setPartitionId(-1);

                    msg.set(batchSize);
                    msg.set(name, true);

                    return msg;
                }

                ClientMessage pncounter_get_encode(const std::string  & name, const std::vector<std::pair<boost::uuids::uuid, int64_t>>  & replicaTimestamps, boost::uuids::uuid targetReplicaUUID) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("PNCounter.Get");

                    msg.setMessageType(static_cast<int32_t>(1900800));
                    msg.setPartitionId(-1);

                    msg.set(targetReplicaUUID);
                    msg.set(name);

                    msg.set(replicaTimestamps, true);

                    return msg;
                }

                ClientMessage pncounter_add_encode(const std::string  & name, int64_t delta, bool getBeforeUpdate, const std::vector<std::pair<boost::uuids::uuid, int64_t>>  & replicaTimestamps, boost::uuids::uuid targetReplicaUUID) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN  + ClientMessage::INT64_SIZE + ClientMessage::UINT8_SIZE + ClientMessage::UUID_SIZE;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(false);
                    msg.setOperationName("PNCounter.Add");

                    msg.setMessageType(static_cast<int32_t>(1901056));
                    msg.setPartitionId(-1);

                    msg.set(delta);
                    msg.set(getBeforeUpdate);
                    msg.set(targetReplicaUUID);
                    msg.set(name);

                    msg.set(replicaTimestamps, true);

                    return msg;
                }

                ClientMessage pncounter_getconfiguredreplicacount_encode(const std::string  & name) {
                    size_t initial_frame_size = ClientMessage::REQUEST_HEADER_LEN ;
                    ClientMessage msg(initial_frame_size);
                    msg.setRetryable(true);
                    msg.setOperationName("PNCounter.GetConfiguredReplicaCount");

                    msg.setMessageType(static_cast<int32_t>(1901312));
                    msg.setPartitionId(-1);

                    msg.set(name, true);

                    return msg;
                }

            }
        }
    }
}

