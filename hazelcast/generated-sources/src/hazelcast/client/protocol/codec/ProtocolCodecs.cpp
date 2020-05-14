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

#include "hazelcast/client/protocol/codec/ProtocolCodecs.h"
#include "hazelcast/client/protocol/EventMessageConst.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/protocol/ResponseMessageConst.h"
#include "hazelcast/client/protocol/codec/StackTraceElement.h"
#include "hazelcast/util/UUID.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddMembershipListenerCodec::REQUEST_TYPE = HZ_CLIENT_ADDMEMBERSHIPLISTENER;
                const bool ClientAddMembershipListenerCodec::RETRYABLE = false;
                const char* ClientAddMembershipListenerCodec::OPERATION_NAME = "Client.AddMembershipListener";
                const ResponseMessageConst ClientAddMembershipListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ClientAddMembershipListenerCodec::encodeRequest(
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAddMembershipListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddMembershipListenerCodec::calculateDataSize(
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ClientAddMembershipListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ClientAddMembershipListenerCodec::ResponseParameters
                ClientAddMembershipListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ClientAddMembershipListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ClientAddMembershipListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddMembershipListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_MEMBER: {
                            Member member = clientMessage->get<Member>();

                            int32_t eventType = clientMessage->get<int32_t>();


                            handleMemberEventV10(member, eventType);
                            break;
                        }
                        case protocol::EVENT_MEMBERLIST: {
                            std::vector<Member> members = clientMessage->getArray<Member>();


                            handleMemberListEventV10(members);
                            break;
                        }
                        case protocol::EVENT_MEMBERATTRIBUTECHANGE: {
                            std::string uuid = clientMessage->get<std::string>();

                            std::string key = clientMessage->get<std::string>();

                            int32_t operationType = clientMessage->get<int32_t>();

                            std::unique_ptr<std::string> value = clientMessage->getNullable<std::string>();


                            handleMemberAttributeChangeEventV10(uuid, key, operationType, value);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ClientAddMembershipListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAddPartitionListenerCodec::REQUEST_TYPE = HZ_CLIENT_ADDPARTITIONLISTENER;
                const bool ClientAddPartitionListenerCodec::RETRYABLE = false;
                const char* ClientAddPartitionListenerCodec::OPERATION_NAME = "Client.AddPartitionListener";
                const ResponseMessageConst ClientAddPartitionListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ClientAddPartitionListenerCodec::encodeRequest() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAddPartitionListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAddPartitionListenerCodec::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }


                //************************ EVENTS START*************************************************************************//
                ClientAddPartitionListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ClientAddPartitionListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_PARTITIONS: {
                            std::vector<std::pair<Address, std::vector<int32_t> > > partitions = clientMessage->getArray<std::pair<Address, std::vector<int32_t> > >();

                            int32_t partitionStateVersion = clientMessage->get<int32_t>();


                            handlePartitionsEventV15(partitions, partitionStateVersion);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ClientAddPartitionListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAuthenticationCodec::REQUEST_TYPE = HZ_CLIENT_AUTHENTICATION;
                const bool ClientAuthenticationCodec::RETRYABLE = true;
                const char* ClientAuthenticationCodec::OPERATION_NAME = "Client.Authentication";
                const ResponseMessageConst ClientAuthenticationCodec::RESPONSE_TYPE = (ResponseMessageConst) 107;

                std::unique_ptr<ClientMessage> ClientAuthenticationCodec::encodeRequest(
                        const std::string &username,
                        const std::string &password,
                        const std::string *uuid,
                        const std::string *ownerUuid,
                        bool isOwnerConnection,
                        const std::string &clientType,
                        uint8_t serializationVersion,
                        const std::string &clientHazelcastVersion) {
                    int32_t requiredDataSize = calculateDataSize(username, password, uuid, ownerUuid, isOwnerConnection,
                                                                 clientType, serializationVersion,
                                                                 clientHazelcastVersion);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAuthenticationCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(username);
                    clientMessage->set(password);
                    clientMessage->set(uuid);
                    clientMessage->set(ownerUuid);
                    clientMessage->set(isOwnerConnection);
                    clientMessage->set(clientType);
                    clientMessage->set(serializationVersion);
                    clientMessage->set(clientHazelcastVersion);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAuthenticationCodec::calculateDataSize(
                        const std::string &username,
                        const std::string &password,
                        const std::string *uuid,
                        const std::string *ownerUuid,
                        bool isOwnerConnection,
                        const std::string &clientType,
                        uint8_t serializationVersion,
                        const std::string &clientHazelcastVersion) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(username);
                    dataSize += ClientMessage::calculateDataSize(password);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(ownerUuid);
                    dataSize += ClientMessage::calculateDataSize(isOwnerConnection);
                    dataSize += ClientMessage::calculateDataSize(clientType);
                    dataSize += ClientMessage::calculateDataSize(serializationVersion);
                    dataSize += ClientMessage::calculateDataSize(clientHazelcastVersion);
                    return dataSize;
                }

                ClientAuthenticationCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    serverHazelcastVersionExist = false;
                    clientUnregisteredMembersExist = false;


                    status = clientMessage.get<uint8_t>();


                    address = clientMessage.getNullable<Address>();


                    uuid = clientMessage.getNullable<std::string>();


                    ownerUuid = clientMessage.getNullable<std::string>();


                    serializationVersion = clientMessage.get<uint8_t>();
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    serverHazelcastVersion = clientMessage.get<std::string>();
                    serverHazelcastVersionExist = true;

                    clientUnregisteredMembers = clientMessage.getNullableArray<Member>();
                    clientUnregisteredMembersExist = true;
                }

                ClientAuthenticationCodec::ResponseParameters
                ClientAuthenticationCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ClientAuthenticationCodec::ResponseParameters(clientMessage);
                }

                ClientAuthenticationCodec::ResponseParameters::ResponseParameters(
                        const ClientAuthenticationCodec::ResponseParameters &rhs) {
                    status = rhs.status;
                    address = std::unique_ptr<Address>(new Address(*rhs.address));
                    uuid = std::unique_ptr<std::string>(new std::string(*rhs.uuid));
                    ownerUuid = std::unique_ptr<std::string>(new std::string(*rhs.ownerUuid));
                    serializationVersion = rhs.serializationVersion;
                    serverHazelcastVersion = rhs.serverHazelcastVersion;
                    clientUnregisteredMembers = std::unique_ptr<std::vector<Member> >(
                            new std::vector<Member>(*rhs.clientUnregisteredMembers));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientAuthenticationCustomCodec::REQUEST_TYPE = HZ_CLIENT_AUTHENTICATIONCUSTOM;
                const bool ClientAuthenticationCustomCodec::RETRYABLE = true;
                const char* ClientAuthenticationCustomCodec::OPERATION_NAME = "Client.AuthenticationCustom";
                const ResponseMessageConst ClientAuthenticationCustomCodec::RESPONSE_TYPE = (ResponseMessageConst) 107;

                std::unique_ptr<ClientMessage> ClientAuthenticationCustomCodec::encodeRequest(
                        const serialization::pimpl::Data &credentials,
                        const std::string *uuid,
                        const std::string *ownerUuid,
                        bool isOwnerConnection,
                        const std::string &clientType,
                        uint8_t serializationVersion,
                        const std::string &clientHazelcastVersion) {
                    int32_t requiredDataSize = calculateDataSize(credentials, uuid, ownerUuid, isOwnerConnection,
                                                                 clientType, serializationVersion,
                                                                 clientHazelcastVersion);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientAuthenticationCustomCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(credentials);
                    clientMessage->set(uuid);
                    clientMessage->set(ownerUuid);
                    clientMessage->set(isOwnerConnection);
                    clientMessage->set(clientType);
                    clientMessage->set(serializationVersion);
                    clientMessage->set(clientHazelcastVersion);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientAuthenticationCustomCodec::calculateDataSize(
                        const serialization::pimpl::Data &credentials,
                        const std::string *uuid,
                        const std::string *ownerUuid,
                        bool isOwnerConnection,
                        const std::string &clientType,
                        uint8_t serializationVersion,
                        const std::string &clientHazelcastVersion) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(credentials);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(ownerUuid);
                    dataSize += ClientMessage::calculateDataSize(isOwnerConnection);
                    dataSize += ClientMessage::calculateDataSize(clientType);
                    dataSize += ClientMessage::calculateDataSize(serializationVersion);
                    dataSize += ClientMessage::calculateDataSize(clientHazelcastVersion);
                    return dataSize;
                }

                ClientAuthenticationCustomCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    serverHazelcastVersionExist = false;
                    clientUnregisteredMembersExist = false;


                    status = clientMessage.get<uint8_t>();


                    address = clientMessage.getNullable<Address>();


                    uuid = clientMessage.getNullable<std::string>();


                    ownerUuid = clientMessage.getNullable<std::string>();


                    serializationVersion = clientMessage.get<uint8_t>();
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    serverHazelcastVersion = clientMessage.get<std::string>();
                    serverHazelcastVersionExist = true;

                    clientUnregisteredMembers = clientMessage.getNullableArray<Member>();
                    clientUnregisteredMembersExist = true;
                }

                ClientAuthenticationCustomCodec::ResponseParameters
                ClientAuthenticationCustomCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ClientAuthenticationCustomCodec::ResponseParameters(clientMessage);
                }

                ClientAuthenticationCustomCodec::ResponseParameters::ResponseParameters(
                        const ClientAuthenticationCustomCodec::ResponseParameters &rhs) {
                    status = rhs.status;
                    address = std::unique_ptr<Address>(new Address(*rhs.address));
                    uuid = std::unique_ptr<std::string>(new std::string(*rhs.uuid));
                    ownerUuid = std::unique_ptr<std::string>(new std::string(*rhs.ownerUuid));
                    serializationVersion = rhs.serializationVersion;
                    serverHazelcastVersion = rhs.serverHazelcastVersion;
                    clientUnregisteredMembers = std::unique_ptr<std::vector<Member> >(
                            new std::vector<Member>(*rhs.clientUnregisteredMembers));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientCreateProxyCodec::REQUEST_TYPE = HZ_CLIENT_CREATEPROXY;
                const bool ClientCreateProxyCodec::RETRYABLE = false;
                const char* ClientCreateProxyCodec::OPERATION_NAME = "Client.CreateProxy";
                const ResponseMessageConst ClientCreateProxyCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ClientCreateProxyCodec::encodeRequest(
                        const std::string &name,
                        const std::string &serviceName,
                        const Address &target) {
                    int32_t requiredDataSize = calculateDataSize(name, serviceName, target);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientCreateProxyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(serviceName);
                    clientMessage->set(target);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientCreateProxyCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &serviceName,
                        const Address &target) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(serviceName);
                    dataSize += ClientMessage::calculateDataSize(target);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientDestroyProxyCodec::REQUEST_TYPE = HZ_CLIENT_DESTROYPROXY;
                const bool ClientDestroyProxyCodec::RETRYABLE = false;
                const char* ClientDestroyProxyCodec::OPERATION_NAME = "Client.DestroyProxy";
                const ResponseMessageConst ClientDestroyProxyCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ClientDestroyProxyCodec::encodeRequest(
                        const std::string &name,
                        const std::string &serviceName) {
                    int32_t requiredDataSize = calculateDataSize(name, serviceName);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientDestroyProxyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(serviceName);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientDestroyProxyCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &serviceName) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(serviceName);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientGetPartitionsCodec::REQUEST_TYPE = HZ_CLIENT_GETPARTITIONS;
                const bool ClientGetPartitionsCodec::RETRYABLE = false;
                const char* ClientGetPartitionsCodec::OPERATION_NAME = "Client.GetPartitions";
                const ResponseMessageConst ClientGetPartitionsCodec::RESPONSE_TYPE = (ResponseMessageConst) 108;

                std::unique_ptr<ClientMessage> ClientGetPartitionsCodec::encodeRequest() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientGetPartitionsCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientGetPartitionsCodec::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }

                ClientGetPartitionsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    partitionStateVersionExist = false;


                    partitions = clientMessage.getArray<std::pair<Address, std::vector<int32_t> > >();
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    partitionStateVersion = clientMessage.get<int32_t>();
                    partitionStateVersionExist = true;
                }

                ClientGetPartitionsCodec::ResponseParameters
                ClientGetPartitionsCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ClientGetPartitionsCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientPingCodec::REQUEST_TYPE = HZ_CLIENT_PING;
                const bool ClientPingCodec::RETRYABLE = true;
                const char* ClientPingCodec::OPERATION_NAME = "Client.Ping";
                const ResponseMessageConst ClientPingCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ClientPingCodec::encodeRequest() {
                    int32_t requiredDataSize = calculateDataSize();
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientPingCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientPingCodec::calculateDataSize() {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ClientMessageType ClientStatisticsCodec::REQUEST_TYPE = HZ_CLIENT_STATISTICS;
                const bool ClientStatisticsCodec::RETRYABLE = false;
                const char* ClientStatisticsCodec::OPERATION_NAME = "Client.Statistics";
                const ResponseMessageConst ClientStatisticsCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ClientStatisticsCodec::encodeRequest(
                        const std::string &stats) {
                    int32_t requiredDataSize = calculateDataSize(stats);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ClientStatisticsCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(stats);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ClientStatisticsCodec::calculateDataSize(
                        const std::string &stats) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(stats);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceCancelOnAddressCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_CANCELONADDRESS;
                const bool ExecutorServiceCancelOnAddressCodec::RETRYABLE = false;
                const char* ExecutorServiceCancelOnAddressCodec::OPERATION_NAME = "ExecutorService.CancelOnAddress";
                const ResponseMessageConst ExecutorServiceCancelOnAddressCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ExecutorServiceCancelOnAddressCodec::encodeRequest(
                        const std::string &uuid,
                        const Address &address,
                        bool interrupt) {
                    int32_t requiredDataSize = calculateDataSize(uuid, address, interrupt);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ExecutorServiceCancelOnAddressCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(uuid);
                    clientMessage->set(address);
                    clientMessage->set(interrupt);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceCancelOnAddressCodec::calculateDataSize(
                        const std::string &uuid,
                        const Address &address,
                        bool interrupt) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(address);
                    dataSize += ClientMessage::calculateDataSize(interrupt);
                    return dataSize;
                }

                ExecutorServiceCancelOnAddressCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ExecutorServiceCancelOnAddressCodec::ResponseParameters
                ExecutorServiceCancelOnAddressCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ExecutorServiceCancelOnAddressCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceCancelOnPartitionCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_CANCELONPARTITION;
                const bool ExecutorServiceCancelOnPartitionCodec::RETRYABLE = false;
                const char* ExecutorServiceCancelOnPartitionCodec::OPERATION_NAME = "ExecutorService.CancelOnPartition";
                const ResponseMessageConst ExecutorServiceCancelOnPartitionCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ExecutorServiceCancelOnPartitionCodec::encodeRequest(
                        const std::string &uuid,
                        int32_t partitionId,
                        bool interrupt) {
                    int32_t requiredDataSize = calculateDataSize(uuid, partitionId, interrupt);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ExecutorServiceCancelOnPartitionCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(uuid);
                    clientMessage->set(partitionId);
                    clientMessage->set(interrupt);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceCancelOnPartitionCodec::calculateDataSize(
                        const std::string &uuid,
                        int32_t partitionId,
                        bool interrupt) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(partitionId);
                    dataSize += ClientMessage::calculateDataSize(interrupt);
                    return dataSize;
                }

                ExecutorServiceCancelOnPartitionCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ExecutorServiceCancelOnPartitionCodec::ResponseParameters
                ExecutorServiceCancelOnPartitionCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ExecutorServiceCancelOnPartitionCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceIsShutdownCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_ISSHUTDOWN;
                const bool ExecutorServiceIsShutdownCodec::RETRYABLE = false;
                const char* ExecutorServiceIsShutdownCodec::OPERATION_NAME = "ExecutorService.IsShutdown";
                const ResponseMessageConst ExecutorServiceIsShutdownCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ExecutorServiceIsShutdownCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ExecutorServiceIsShutdownCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceIsShutdownCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ExecutorServiceIsShutdownCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ExecutorServiceIsShutdownCodec::ResponseParameters
                ExecutorServiceIsShutdownCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ExecutorServiceIsShutdownCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceShutdownCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_SHUTDOWN;
                const bool ExecutorServiceShutdownCodec::RETRYABLE = false;
                const char* ExecutorServiceShutdownCodec::OPERATION_NAME = "ExecutorService.Shutdown";
                const ResponseMessageConst ExecutorServiceShutdownCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ExecutorServiceShutdownCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ExecutorServiceShutdownCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceShutdownCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceSubmitToAddressCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_SUBMITTOADDRESS;
                const bool ExecutorServiceSubmitToAddressCodec::RETRYABLE = false;
                const char* ExecutorServiceSubmitToAddressCodec::OPERATION_NAME = "ExecutorService.SubmitToAddress";
                const ResponseMessageConst ExecutorServiceSubmitToAddressCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ExecutorServiceSubmitToAddressCodec::encodeRequest(
                        const std::string &name,
                        const std::string &uuid,
                        const serialization::pimpl::Data &callable,
                        const Address &address) {
                    int32_t requiredDataSize = calculateDataSize(name, uuid, callable, address);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ExecutorServiceSubmitToAddressCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(uuid);
                    clientMessage->set(callable);
                    clientMessage->set(address);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceSubmitToAddressCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &uuid,
                        const serialization::pimpl::Data &callable,
                        const Address &address) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(callable);
                    dataSize += ClientMessage::calculateDataSize(address);
                    return dataSize;
                }

                ExecutorServiceSubmitToAddressCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ExecutorServiceSubmitToAddressCodec::ResponseParameters
                ExecutorServiceSubmitToAddressCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ExecutorServiceSubmitToAddressCodec::ResponseParameters(clientMessage);
                }

                ExecutorServiceSubmitToAddressCodec::ResponseParameters::ResponseParameters(
                        const ExecutorServiceSubmitToAddressCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ExecutorServiceMessageType ExecutorServiceSubmitToPartitionCodec::REQUEST_TYPE = HZ_EXECUTORSERVICE_SUBMITTOPARTITION;
                const bool ExecutorServiceSubmitToPartitionCodec::RETRYABLE = false;
                const char* ExecutorServiceSubmitToPartitionCodec::OPERATION_NAME = "ExecutorService.SubmitToPartition";
                const ResponseMessageConst ExecutorServiceSubmitToPartitionCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ExecutorServiceSubmitToPartitionCodec::encodeRequest(
                        const std::string &name,
                        const std::string &uuid,
                        const serialization::pimpl::Data &callable,
                        int32_t partitionId) {
                    int32_t requiredDataSize = calculateDataSize(name, uuid, callable, partitionId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ExecutorServiceSubmitToPartitionCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(uuid);
                    clientMessage->set(callable);
                    clientMessage->set(partitionId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ExecutorServiceSubmitToPartitionCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &uuid,
                        const serialization::pimpl::Data &callable,
                        int32_t partitionId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(uuid);
                    dataSize += ClientMessage::calculateDataSize(callable);
                    dataSize += ClientMessage::calculateDataSize(partitionId);
                    return dataSize;
                }

                ExecutorServiceSubmitToPartitionCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ExecutorServiceSubmitToPartitionCodec::ResponseParameters
                ExecutorServiceSubmitToPartitionCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ExecutorServiceSubmitToPartitionCodec::ResponseParameters(clientMessage);
                }

                ExecutorServiceSubmitToPartitionCodec::ResponseParameters::ResponseParameters(
                        const ExecutorServiceSubmitToPartitionCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const FlakeIdGeneratorMessageType FlakeIdGeneratorNewIdBatchCodec::REQUEST_TYPE = HZ_FLAKEIDGENERATOR_NEWIDBATCH;
                const bool FlakeIdGeneratorNewIdBatchCodec::RETRYABLE = true;
                const char* FlakeIdGeneratorNewIdBatchCodec::OPERATION_NAME = "FlakeIdGenerator.NewIdBatch";
                const ResponseMessageConst FlakeIdGeneratorNewIdBatchCodec::RESPONSE_TYPE = (ResponseMessageConst) 126;

                std::unique_ptr<ClientMessage> FlakeIdGeneratorNewIdBatchCodec::encodeRequest(
                        const std::string &name,
                        int32_t batchSize) {
                    int32_t requiredDataSize = calculateDataSize(name, batchSize);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) FlakeIdGeneratorNewIdBatchCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(batchSize);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t FlakeIdGeneratorNewIdBatchCodec::calculateDataSize(
                        const std::string &name,
                        int32_t batchSize) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(batchSize);
                    return dataSize;
                }

                FlakeIdGeneratorNewIdBatchCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    base = clientMessage.get<int64_t>();


                    increment = clientMessage.get<int64_t>();


                    batchSize = clientMessage.get<int32_t>();

                }

                FlakeIdGeneratorNewIdBatchCodec::ResponseParameters
                FlakeIdGeneratorNewIdBatchCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return FlakeIdGeneratorNewIdBatchCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListAddAllCodec::REQUEST_TYPE = HZ_LIST_ADDALL;
                const bool ListAddAllCodec::RETRYABLE = false;
                const char* ListAddAllCodec::OPERATION_NAME = "List.AddAll";
                const ResponseMessageConst ListAddAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListAddAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList) {
                    int32_t requiredDataSize = calculateDataSize(name, valueList);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListAddAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(valueList);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListAddAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(valueList);
                    return dataSize;
                }

                ListAddAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListAddAllCodec::ResponseParameters
                ListAddAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListAddAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListAddAllWithIndexCodec::REQUEST_TYPE = HZ_LIST_ADDALLWITHINDEX;
                const bool ListAddAllWithIndexCodec::RETRYABLE = false;
                const char* ListAddAllWithIndexCodec::OPERATION_NAME = "List.AddAllWithIndex";
                const ResponseMessageConst ListAddAllWithIndexCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListAddAllWithIndexCodec::encodeRequest(
                        const std::string &name,
                        int32_t index,
                        const std::vector<serialization::pimpl::Data> &valueList) {
                    int32_t requiredDataSize = calculateDataSize(name, index, valueList);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListAddAllWithIndexCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(index);
                    clientMessage->setArray<serialization::pimpl::Data>(valueList);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListAddAllWithIndexCodec::calculateDataSize(
                        const std::string &name,
                        int32_t index,
                        const std::vector<serialization::pimpl::Data> &valueList) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(index);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(valueList);
                    return dataSize;
                }

                ListAddAllWithIndexCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListAddAllWithIndexCodec::ResponseParameters
                ListAddAllWithIndexCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListAddAllWithIndexCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListAddCodec::REQUEST_TYPE = HZ_LIST_ADD;
                const bool ListAddCodec::RETRYABLE = false;
                const char* ListAddCodec::OPERATION_NAME = "List.Add";
                const ResponseMessageConst ListAddCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListAddCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListAddCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListAddCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListAddCodec::ResponseParameters
                ListAddCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListAddCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListAddListenerCodec::REQUEST_TYPE = HZ_LIST_ADDLISTENER;
                const bool ListAddListenerCodec::RETRYABLE = false;
                const char* ListAddListenerCodec::OPERATION_NAME = "List.AddListener";
                const ResponseMessageConst ListAddListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ListAddListenerCodec::encodeRequest(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListAddListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListAddListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ListAddListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ListAddListenerCodec::ResponseParameters
                ListAddListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListAddListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ListAddListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ListAddListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ITEM: {
                            std::unique_ptr<serialization::pimpl::Data> item = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t eventType = clientMessage->get<int32_t>();


                            handleItemEventV10(item, uuid, eventType);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ListAddListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListAddWithIndexCodec::REQUEST_TYPE = HZ_LIST_ADDWITHINDEX;
                const bool ListAddWithIndexCodec::RETRYABLE = false;
                const char* ListAddWithIndexCodec::OPERATION_NAME = "List.AddWithIndex";
                const ResponseMessageConst ListAddWithIndexCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ListAddWithIndexCodec::encodeRequest(
                        const std::string &name,
                        int32_t index,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, index, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListAddWithIndexCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(index);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListAddWithIndexCodec::calculateDataSize(
                        const std::string &name,
                        int32_t index,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(index);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListClearCodec::REQUEST_TYPE = HZ_LIST_CLEAR;
                const bool ListClearCodec::RETRYABLE = false;
                const char* ListClearCodec::OPERATION_NAME = "List.Clear";
                const ResponseMessageConst ListClearCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ListClearCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListClearCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListClearCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListCompareAndRemoveAllCodec::REQUEST_TYPE = HZ_LIST_COMPAREANDREMOVEALL;
                const bool ListCompareAndRemoveAllCodec::RETRYABLE = false;
                const char* ListCompareAndRemoveAllCodec::OPERATION_NAME = "List.CompareAndRemoveAll";
                const ResponseMessageConst ListCompareAndRemoveAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListCompareAndRemoveAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t requiredDataSize = calculateDataSize(name, values);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListCompareAndRemoveAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(values);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListCompareAndRemoveAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(values);
                    return dataSize;
                }

                ListCompareAndRemoveAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListCompareAndRemoveAllCodec::ResponseParameters
                ListCompareAndRemoveAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListCompareAndRemoveAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListCompareAndRetainAllCodec::REQUEST_TYPE = HZ_LIST_COMPAREANDRETAINALL;
                const bool ListCompareAndRetainAllCodec::RETRYABLE = false;
                const char* ListCompareAndRetainAllCodec::OPERATION_NAME = "List.CompareAndRetainAll";
                const ResponseMessageConst ListCompareAndRetainAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListCompareAndRetainAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t requiredDataSize = calculateDataSize(name, values);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListCompareAndRetainAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(values);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListCompareAndRetainAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(values);
                    return dataSize;
                }

                ListCompareAndRetainAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListCompareAndRetainAllCodec::ResponseParameters
                ListCompareAndRetainAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListCompareAndRetainAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListContainsAllCodec::REQUEST_TYPE = HZ_LIST_CONTAINSALL;
                const bool ListContainsAllCodec::RETRYABLE = true;
                const char* ListContainsAllCodec::OPERATION_NAME = "List.ContainsAll";
                const ResponseMessageConst ListContainsAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListContainsAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t requiredDataSize = calculateDataSize(name, values);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListContainsAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(values);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListContainsAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(values);
                    return dataSize;
                }

                ListContainsAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListContainsAllCodec::ResponseParameters
                ListContainsAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListContainsAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListContainsCodec::REQUEST_TYPE = HZ_LIST_CONTAINS;
                const bool ListContainsCodec::RETRYABLE = true;
                const char* ListContainsCodec::OPERATION_NAME = "List.Contains";
                const ResponseMessageConst ListContainsCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListContainsCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListContainsCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListContainsCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListContainsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListContainsCodec::ResponseParameters
                ListContainsCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListContainsCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListGetAllCodec::REQUEST_TYPE = HZ_LIST_GETALL;
                const bool ListGetAllCodec::RETRYABLE = true;
                const char* ListGetAllCodec::OPERATION_NAME = "List.GetAll";
                const ResponseMessageConst ListGetAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> ListGetAllCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListGetAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListGetAllCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ListGetAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                ListGetAllCodec::ResponseParameters
                ListGetAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListGetAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListGetCodec::REQUEST_TYPE = HZ_LIST_GET;
                const bool ListGetCodec::RETRYABLE = true;
                const char* ListGetCodec::OPERATION_NAME = "List.Get";
                const ResponseMessageConst ListGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ListGetCodec::encodeRequest(
                        const std::string &name,
                        int32_t index) {
                    int32_t requiredDataSize = calculateDataSize(name, index);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListGetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(index);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListGetCodec::calculateDataSize(
                        const std::string &name,
                        int32_t index) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(index);
                    return dataSize;
                }

                ListGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ListGetCodec::ResponseParameters
                ListGetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListGetCodec::ResponseParameters(clientMessage);
                }

                ListGetCodec::ResponseParameters::ResponseParameters(const ListGetCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListIndexOfCodec::REQUEST_TYPE = HZ_LIST_INDEXOF;
                const bool ListIndexOfCodec::RETRYABLE = true;
                const char* ListIndexOfCodec::OPERATION_NAME = "List.IndexOf";
                const ResponseMessageConst ListIndexOfCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> ListIndexOfCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListIndexOfCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListIndexOfCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListIndexOfCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                ListIndexOfCodec::ResponseParameters
                ListIndexOfCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListIndexOfCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListIsEmptyCodec::REQUEST_TYPE = HZ_LIST_ISEMPTY;
                const bool ListIsEmptyCodec::RETRYABLE = true;
                const char* ListIsEmptyCodec::OPERATION_NAME = "List.IsEmpty";
                const ResponseMessageConst ListIsEmptyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListIsEmptyCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListIsEmptyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListIsEmptyCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ListIsEmptyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListIsEmptyCodec::ResponseParameters
                ListIsEmptyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListIsEmptyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListLastIndexOfCodec::REQUEST_TYPE = HZ_LIST_LASTINDEXOF;
                const bool ListLastIndexOfCodec::RETRYABLE = true;
                const char* ListLastIndexOfCodec::OPERATION_NAME = "List.LastIndexOf";
                const ResponseMessageConst ListLastIndexOfCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> ListLastIndexOfCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListLastIndexOfCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListLastIndexOfCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListLastIndexOfCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                ListLastIndexOfCodec::ResponseParameters
                ListLastIndexOfCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListLastIndexOfCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListRemoveCodec::REQUEST_TYPE = HZ_LIST_REMOVE;
                const bool ListRemoveCodec::RETRYABLE = false;
                const char* ListRemoveCodec::OPERATION_NAME = "List.Remove";
                const ResponseMessageConst ListRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListRemoveCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListRemoveCodec::ResponseParameters
                ListRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListRemoveListenerCodec::REQUEST_TYPE = HZ_LIST_REMOVELISTENER;
                const bool ListRemoveListenerCodec::RETRYABLE = true;
                const char* ListRemoveListenerCodec::OPERATION_NAME = "List.RemoveListener";
                const ResponseMessageConst ListRemoveListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ListRemoveListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListRemoveListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListRemoveListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                ListRemoveListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ListRemoveListenerCodec::ResponseParameters
                ListRemoveListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListRemoveListenerCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListRemoveWithIndexCodec::REQUEST_TYPE = HZ_LIST_REMOVEWITHINDEX;
                const bool ListRemoveWithIndexCodec::RETRYABLE = false;
                const char* ListRemoveWithIndexCodec::OPERATION_NAME = "List.RemoveWithIndex";
                const ResponseMessageConst ListRemoveWithIndexCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ListRemoveWithIndexCodec::encodeRequest(
                        const std::string &name,
                        int32_t index) {
                    int32_t requiredDataSize = calculateDataSize(name, index);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListRemoveWithIndexCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(index);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListRemoveWithIndexCodec::calculateDataSize(
                        const std::string &name,
                        int32_t index) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(index);
                    return dataSize;
                }

                ListRemoveWithIndexCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ListRemoveWithIndexCodec::ResponseParameters
                ListRemoveWithIndexCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListRemoveWithIndexCodec::ResponseParameters(clientMessage);
                }

                ListRemoveWithIndexCodec::ResponseParameters::ResponseParameters(
                        const ListRemoveWithIndexCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListSetCodec::REQUEST_TYPE = HZ_LIST_SET;
                const bool ListSetCodec::RETRYABLE = false;
                const char* ListSetCodec::OPERATION_NAME = "List.Set";
                const ResponseMessageConst ListSetCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ListSetCodec::encodeRequest(
                        const std::string &name,
                        int32_t index,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, index, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListSetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(index);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListSetCodec::calculateDataSize(
                        const std::string &name,
                        int32_t index,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(index);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ListSetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ListSetCodec::ResponseParameters
                ListSetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListSetCodec::ResponseParameters(clientMessage);
                }

                ListSetCodec::ResponseParameters::ResponseParameters(const ListSetCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListSizeCodec::REQUEST_TYPE = HZ_LIST_SIZE;
                const bool ListSizeCodec::RETRYABLE = true;
                const char* ListSizeCodec::OPERATION_NAME = "List.Size";
                const ResponseMessageConst ListSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> ListSizeCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListSizeCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ListSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                ListSizeCodec::ResponseParameters
                ListSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ListMessageType ListSubCodec::REQUEST_TYPE = HZ_LIST_SUB;
                const bool ListSubCodec::RETRYABLE = true;
                const char* ListSubCodec::OPERATION_NAME = "List.Sub";
                const ResponseMessageConst ListSubCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> ListSubCodec::encodeRequest(
                        const std::string &name,
                        int32_t from,
                        int32_t to) {
                    int32_t requiredDataSize = calculateDataSize(name, from, to);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ListSubCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(from);
                    clientMessage->set(to);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ListSubCodec::calculateDataSize(
                        const std::string &name,
                        int32_t from,
                        int32_t to) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(from);
                    dataSize += ClientMessage::calculateDataSize(to);
                    return dataSize;
                }

                ListSubCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                ListSubCodec::ResponseParameters
                ListSubCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ListSubCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddEntryListenerCodec::REQUEST_TYPE = HZ_MAP_ADDENTRYLISTENER;
                const bool MapAddEntryListenerCodec::RETRYABLE = false;
                const char* MapAddEntryListenerCodec::OPERATION_NAME = "Map.AddEntryListener";
                const ResponseMessageConst MapAddEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> MapAddEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, listenerFlags, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(listenerFlags);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(listenerFlags);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddEntryListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MapAddEntryListenerCodec::ResponseParameters
                MapAddEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapAddEntryListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                MapAddEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddEntryListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[MapAddEntryListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddEntryListenerToKeyCodec::REQUEST_TYPE = HZ_MAP_ADDENTRYLISTENERTOKEY;
                const bool MapAddEntryListenerToKeyCodec::RETRYABLE = false;
                const char* MapAddEntryListenerToKeyCodec::OPERATION_NAME = "Map.AddEntryListenerToKey";
                const ResponseMessageConst MapAddEntryListenerToKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> MapAddEntryListenerToKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, key, includeValue, listenerFlags, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddEntryListenerToKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(includeValue);
                    clientMessage->set(listenerFlags);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddEntryListenerToKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(listenerFlags);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddEntryListenerToKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MapAddEntryListenerToKeyCodec::ResponseParameters
                MapAddEntryListenerToKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapAddEntryListenerToKeyCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                MapAddEntryListenerToKeyCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddEntryListenerToKeyCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[MapAddEntryListenerToKeyCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddEntryListenerWithPredicateCodec::REQUEST_TYPE = HZ_MAP_ADDENTRYLISTENERWITHPREDICATE;
                const bool MapAddEntryListenerWithPredicateCodec::RETRYABLE = false;
                const char* MapAddEntryListenerWithPredicateCodec::OPERATION_NAME = "Map.AddEntryListenerWithPredicate";
                const ResponseMessageConst MapAddEntryListenerWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> MapAddEntryListenerWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate, includeValue, listenerFlags,
                                                                 localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddEntryListenerWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->set(includeValue);
                    clientMessage->set(listenerFlags);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddEntryListenerWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate,
                        bool includeValue,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(listenerFlags);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddEntryListenerWithPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MapAddEntryListenerWithPredicateCodec::ResponseParameters
                MapAddEntryListenerWithPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapAddEntryListenerWithPredicateCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                MapAddEntryListenerWithPredicateCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddEntryListenerWithPredicateCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[MapAddEntryListenerWithPredicateCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddIndexCodec::REQUEST_TYPE = HZ_MAP_ADDINDEX;
                const bool MapAddIndexCodec::RETRYABLE = false;
                const char* MapAddIndexCodec::OPERATION_NAME = "Map.AddIndex";
                const ResponseMessageConst MapAddIndexCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapAddIndexCodec::encodeRequest(
                        const std::string &name,
                        const std::string &attribute,
                        bool ordered) {
                    int32_t requiredDataSize = calculateDataSize(name, attribute, ordered);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddIndexCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(attribute);
                    clientMessage->set(ordered);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddIndexCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &attribute,
                        bool ordered) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(attribute);
                    dataSize += ClientMessage::calculateDataSize(ordered);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddInterceptorCodec::REQUEST_TYPE = HZ_MAP_ADDINTERCEPTOR;
                const bool MapAddInterceptorCodec::RETRYABLE = false;
                const char* MapAddInterceptorCodec::OPERATION_NAME = "Map.AddInterceptor";
                const ResponseMessageConst MapAddInterceptorCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> MapAddInterceptorCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &interceptor) {
                    int32_t requiredDataSize = calculateDataSize(name, interceptor);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddInterceptorCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(interceptor);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddInterceptorCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &interceptor) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(interceptor);
                    return dataSize;
                }

                MapAddInterceptorCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MapAddInterceptorCodec::ResponseParameters
                MapAddInterceptorCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapAddInterceptorCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapAddNearCacheEntryListenerCodec::REQUEST_TYPE = HZ_MAP_ADDNEARCACHEENTRYLISTENER;
                const bool MapAddNearCacheEntryListenerCodec::RETRYABLE = false;
                const char* MapAddNearCacheEntryListenerCodec::OPERATION_NAME = "Map.AddNearCacheEntryListener";
                const ResponseMessageConst MapAddNearCacheEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> MapAddNearCacheEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, listenerFlags, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapAddNearCacheEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(listenerFlags);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapAddNearCacheEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        int32_t listenerFlags,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(listenerFlags);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MapAddNearCacheEntryListenerCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MapAddNearCacheEntryListenerCodec::ResponseParameters
                MapAddNearCacheEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapAddNearCacheEntryListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                MapAddNearCacheEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_IMAPINVALIDATION: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            if (clientMessage->isComplete()) {
                                handleIMapInvalidationEventV10(key);
                                return;
                            }
                            std::string sourceUuid = clientMessage->get<std::string>();

                            util::UUID partitionUuid = clientMessage->get<util::UUID>();

                            int64_t sequence = clientMessage->get<int64_t>();


                            handleIMapInvalidationEventV14(key, sourceUuid, partitionUuid, sequence);
                            break;
                        }
                        case protocol::EVENT_IMAPBATCHINVALIDATION: {
                            std::vector<serialization::pimpl::Data> keys = clientMessage->getArray<serialization::pimpl::Data>();

                            if (clientMessage->isComplete()) {
                                handleIMapBatchInvalidationEventV10(keys);
                                return;
                            }
                            std::vector<std::string> sourceUuids = clientMessage->getArray<std::string>();

                            std::vector<util::UUID> partitionUuids = clientMessage->getArray<util::UUID>();

                            std::vector<int64_t> sequences = clientMessage->getArray<int64_t>();


                            handleIMapBatchInvalidationEventV14(keys, sourceUuids, partitionUuids, sequences);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[MapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapClearCodec::REQUEST_TYPE = HZ_MAP_CLEAR;
                const bool MapClearCodec::RETRYABLE = false;
                const char* MapClearCodec::OPERATION_NAME = "Map.Clear";
                const ResponseMessageConst MapClearCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapClearCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapClearCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapClearCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapContainsKeyCodec::REQUEST_TYPE = HZ_MAP_CONTAINSKEY;
                const bool MapContainsKeyCodec::RETRYABLE = true;
                const char* MapContainsKeyCodec::OPERATION_NAME = "Map.ContainsKey";
                const ResponseMessageConst MapContainsKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapContainsKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapContainsKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapContainsKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapContainsKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapContainsKeyCodec::ResponseParameters
                MapContainsKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapContainsKeyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapContainsValueCodec::REQUEST_TYPE = HZ_MAP_CONTAINSVALUE;
                const bool MapContainsValueCodec::RETRYABLE = true;
                const char* MapContainsValueCodec::OPERATION_NAME = "Map.ContainsValue";
                const ResponseMessageConst MapContainsValueCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapContainsValueCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapContainsValueCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapContainsValueCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                MapContainsValueCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapContainsValueCodec::ResponseParameters
                MapContainsValueCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapContainsValueCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapDeleteCodec::REQUEST_TYPE = HZ_MAP_DELETE;
                const bool MapDeleteCodec::RETRYABLE = false;
                const char* MapDeleteCodec::OPERATION_NAME = "Map.Delete";
                const ResponseMessageConst MapDeleteCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapDeleteCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapDeleteCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapDeleteCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapEntriesWithPagingPredicateCodec::REQUEST_TYPE = HZ_MAP_ENTRIESWITHPAGINGPREDICATE;
                const bool MapEntriesWithPagingPredicateCodec::RETRYABLE = true;
                const char* MapEntriesWithPagingPredicateCodec::OPERATION_NAME = "Map.EntriesWithPagingPredicate";
                const ResponseMessageConst MapEntriesWithPagingPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapEntriesWithPagingPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapEntriesWithPagingPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapEntriesWithPagingPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapEntriesWithPagingPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapEntriesWithPagingPredicateCodec::ResponseParameters
                MapEntriesWithPagingPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapEntriesWithPagingPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapEntriesWithPredicateCodec::REQUEST_TYPE = HZ_MAP_ENTRIESWITHPREDICATE;
                const bool MapEntriesWithPredicateCodec::RETRYABLE = true;
                const char* MapEntriesWithPredicateCodec::OPERATION_NAME = "Map.EntriesWithPredicate";
                const ResponseMessageConst MapEntriesWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapEntriesWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapEntriesWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapEntriesWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapEntriesWithPredicateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapEntriesWithPredicateCodec::ResponseParameters
                MapEntriesWithPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapEntriesWithPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapEntrySetCodec::REQUEST_TYPE = HZ_MAP_ENTRYSET;
                const bool MapEntrySetCodec::RETRYABLE = true;
                const char* MapEntrySetCodec::OPERATION_NAME = "Map.EntrySet";
                const ResponseMessageConst MapEntrySetCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapEntrySetCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapEntrySetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapEntrySetCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MapEntrySetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapEntrySetCodec::ResponseParameters
                MapEntrySetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapEntrySetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapEvictAllCodec::REQUEST_TYPE = HZ_MAP_EVICTALL;
                const bool MapEvictAllCodec::RETRYABLE = false;
                const char* MapEvictAllCodec::OPERATION_NAME = "Map.EvictAll";
                const ResponseMessageConst MapEvictAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapEvictAllCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapEvictAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapEvictAllCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapEvictCodec::REQUEST_TYPE = HZ_MAP_EVICT;
                const bool MapEvictCodec::RETRYABLE = false;
                const char* MapEvictCodec::OPERATION_NAME = "Map.Evict";
                const ResponseMessageConst MapEvictCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapEvictCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapEvictCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapEvictCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapEvictCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapEvictCodec::ResponseParameters
                MapEvictCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapEvictCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapExecuteOnAllKeysCodec::REQUEST_TYPE = HZ_MAP_EXECUTEONALLKEYS;
                const bool MapExecuteOnAllKeysCodec::RETRYABLE = false;
                const char* MapExecuteOnAllKeysCodec::OPERATION_NAME = "Map.ExecuteOnAllKeys";
                const ResponseMessageConst MapExecuteOnAllKeysCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapExecuteOnAllKeysCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor) {
                    int32_t requiredDataSize = calculateDataSize(name, entryProcessor);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapExecuteOnAllKeysCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(entryProcessor);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapExecuteOnAllKeysCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(entryProcessor);
                    return dataSize;
                }

                MapExecuteOnAllKeysCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapExecuteOnAllKeysCodec::ResponseParameters
                MapExecuteOnAllKeysCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapExecuteOnAllKeysCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapExecuteOnKeyCodec::REQUEST_TYPE = HZ_MAP_EXECUTEONKEY;
                const bool MapExecuteOnKeyCodec::RETRYABLE = false;
                const char* MapExecuteOnKeyCodec::OPERATION_NAME = "Map.ExecuteOnKey";
                const ResponseMessageConst MapExecuteOnKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapExecuteOnKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, entryProcessor, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapExecuteOnKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(entryProcessor);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapExecuteOnKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(entryProcessor);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapExecuteOnKeyCodec::ResponseParameters::ResponseParameters(ClientMessage clientMessage) {
                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapExecuteOnKeyCodec::ResponseParameters
                MapExecuteOnKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapExecuteOnKeyCodec::ResponseParameters(clientMessage);
                }

                MapExecuteOnKeyCodec::ResponseParameters::ResponseParameters(
                        const MapExecuteOnKeyCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapExecuteOnKeysCodec::REQUEST_TYPE = HZ_MAP_EXECUTEONKEYS;
                const bool MapExecuteOnKeysCodec::RETRYABLE = false;
                const char* MapExecuteOnKeysCodec::OPERATION_NAME = "Map.ExecuteOnKeys";
                const ResponseMessageConst MapExecuteOnKeysCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapExecuteOnKeysCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const std::vector<serialization::pimpl::Data> &keys) {
                    int32_t requiredDataSize = calculateDataSize(name, entryProcessor, keys);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapExecuteOnKeysCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(entryProcessor);
                    clientMessage->setArray<serialization::pimpl::Data>(keys);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapExecuteOnKeysCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const std::vector<serialization::pimpl::Data> &keys) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(entryProcessor);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(keys);
                    return dataSize;
                }

                MapExecuteOnKeysCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapExecuteOnKeysCodec::ResponseParameters
                MapExecuteOnKeysCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapExecuteOnKeysCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapExecuteWithPredicateCodec::REQUEST_TYPE = HZ_MAP_EXECUTEWITHPREDICATE;
                const bool MapExecuteWithPredicateCodec::RETRYABLE = false;
                const char* MapExecuteWithPredicateCodec::OPERATION_NAME = "Map.ExecuteWithPredicate";
                const ResponseMessageConst MapExecuteWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapExecuteWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, entryProcessor, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapExecuteWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(entryProcessor);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapExecuteWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(entryProcessor);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapExecuteWithPredicateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapExecuteWithPredicateCodec::ResponseParameters
                MapExecuteWithPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapExecuteWithPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapFlushCodec::REQUEST_TYPE = HZ_MAP_FLUSH;
                const bool MapFlushCodec::RETRYABLE = false;
                const char* MapFlushCodec::OPERATION_NAME = "Map.Flush";
                const ResponseMessageConst MapFlushCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapFlushCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapFlushCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapFlushCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapForceUnlockCodec::REQUEST_TYPE = HZ_MAP_FORCEUNLOCK;
                const bool MapForceUnlockCodec::RETRYABLE = true;
                const char* MapForceUnlockCodec::OPERATION_NAME = "Map.ForceUnlock";
                const ResponseMessageConst MapForceUnlockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapForceUnlockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapForceUnlockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapForceUnlockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapGetAllCodec::REQUEST_TYPE = HZ_MAP_GETALL;
                const bool MapGetAllCodec::RETRYABLE = false;
                const char* MapGetAllCodec::OPERATION_NAME = "Map.GetAll";
                const ResponseMessageConst MapGetAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapGetAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &keys) {
                    int32_t requiredDataSize = calculateDataSize(name, keys);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapGetAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(keys);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapGetAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &keys) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(keys);
                    return dataSize;
                }

                MapGetAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapGetAllCodec::ResponseParameters
                MapGetAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapGetAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapGetCodec::REQUEST_TYPE = HZ_MAP_GET;
                const bool MapGetCodec::RETRYABLE = true;
                const char* MapGetCodec::OPERATION_NAME = "Map.Get";
                const ResponseMessageConst MapGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapGetCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapGetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapGetCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapGetCodec::ResponseParameters MapGetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapGetCodec::ResponseParameters(clientMessage);
                }

                MapGetCodec::ResponseParameters::ResponseParameters(const MapGetCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapGetEntryViewCodec::REQUEST_TYPE = HZ_MAP_GETENTRYVIEW;
                const bool MapGetEntryViewCodec::RETRYABLE = true;
                const char* MapGetEntryViewCodec::OPERATION_NAME = "Map.GetEntryView";
                const ResponseMessageConst MapGetEntryViewCodec::RESPONSE_TYPE = (ResponseMessageConst) 111;

                std::unique_ptr<ClientMessage> MapGetEntryViewCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapGetEntryViewCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapGetEntryViewCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapGetEntryViewCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<map::DataEntryView>();

                }

                MapGetEntryViewCodec::ResponseParameters
                MapGetEntryViewCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapGetEntryViewCodec::ResponseParameters(clientMessage);
                }

                MapGetEntryViewCodec::ResponseParameters::ResponseParameters(
                        const MapGetEntryViewCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<map::DataEntryView>(new map::DataEntryView(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapIsEmptyCodec::REQUEST_TYPE = HZ_MAP_ISEMPTY;
                const bool MapIsEmptyCodec::RETRYABLE = true;
                const char* MapIsEmptyCodec::OPERATION_NAME = "Map.IsEmpty";
                const ResponseMessageConst MapIsEmptyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapIsEmptyCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapIsEmptyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapIsEmptyCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MapIsEmptyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapIsEmptyCodec::ResponseParameters
                MapIsEmptyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapIsEmptyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapIsLockedCodec::REQUEST_TYPE = HZ_MAP_ISLOCKED;
                const bool MapIsLockedCodec::RETRYABLE = true;
                const char* MapIsLockedCodec::OPERATION_NAME = "Map.IsLocked";
                const ResponseMessageConst MapIsLockedCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapIsLockedCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapIsLockedCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapIsLockedCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                MapIsLockedCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapIsLockedCodec::ResponseParameters
                MapIsLockedCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapIsLockedCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapKeySetCodec::REQUEST_TYPE = HZ_MAP_KEYSET;
                const bool MapKeySetCodec::RETRYABLE = true;
                const char* MapKeySetCodec::OPERATION_NAME = "Map.KeySet";
                const ResponseMessageConst MapKeySetCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MapKeySetCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapKeySetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapKeySetCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MapKeySetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MapKeySetCodec::ResponseParameters
                MapKeySetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapKeySetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapKeySetWithPagingPredicateCodec::REQUEST_TYPE = HZ_MAP_KEYSETWITHPAGINGPREDICATE;
                const bool MapKeySetWithPagingPredicateCodec::RETRYABLE = true;
                const char* MapKeySetWithPagingPredicateCodec::OPERATION_NAME = "Map.KeySetWithPagingPredicate";
                const ResponseMessageConst MapKeySetWithPagingPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MapKeySetWithPagingPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapKeySetWithPagingPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapKeySetWithPagingPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapKeySetWithPagingPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MapKeySetWithPagingPredicateCodec::ResponseParameters
                MapKeySetWithPagingPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapKeySetWithPagingPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapKeySetWithPredicateCodec::REQUEST_TYPE = HZ_MAP_KEYSETWITHPREDICATE;
                const bool MapKeySetWithPredicateCodec::RETRYABLE = true;
                const char* MapKeySetWithPredicateCodec::OPERATION_NAME = "Map.KeySetWithPredicate";
                const ResponseMessageConst MapKeySetWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MapKeySetWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapKeySetWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapKeySetWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapKeySetWithPredicateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MapKeySetWithPredicateCodec::ResponseParameters
                MapKeySetWithPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapKeySetWithPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapLockCodec::REQUEST_TYPE = HZ_MAP_LOCK;
                const bool MapLockCodec::RETRYABLE = true;
                const char* MapLockCodec::OPERATION_NAME = "Map.Lock";
                const ResponseMessageConst MapLockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapLockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, ttl, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapLockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapLockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutAllCodec::REQUEST_TYPE = HZ_MAP_PUTALL;
                const bool MapPutAllCodec::RETRYABLE = false;
                const char* MapPutAllCodec::OPERATION_NAME = "Map.PutAll";
                const ResponseMessageConst MapPutAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapPutAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries) {
                    int32_t requiredDataSize = calculateDataSize(name, entries);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapPutAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >(
                            entries);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >(
                            entries);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutCodec::REQUEST_TYPE = HZ_MAP_PUT;
                const bool MapPutCodec::RETRYABLE = false;
                const char* MapPutCodec::OPERATION_NAME = "Map.Put";
                const ResponseMessageConst MapPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapPutCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, ttl);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapPutCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }

                MapPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapPutCodec::ResponseParameters MapPutCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapPutCodec::ResponseParameters(clientMessage);
                }

                MapPutCodec::ResponseParameters::ResponseParameters(const MapPutCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutIfAbsentCodec::REQUEST_TYPE = HZ_MAP_PUTIFABSENT;
                const bool MapPutIfAbsentCodec::RETRYABLE = false;
                const char* MapPutIfAbsentCodec::OPERATION_NAME = "Map.PutIfAbsent";
                const ResponseMessageConst MapPutIfAbsentCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapPutIfAbsentCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, ttl);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapPutIfAbsentCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutIfAbsentCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }

                MapPutIfAbsentCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapPutIfAbsentCodec::ResponseParameters
                MapPutIfAbsentCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapPutIfAbsentCodec::ResponseParameters(clientMessage);
                }

                MapPutIfAbsentCodec::ResponseParameters::ResponseParameters(
                        const MapPutIfAbsentCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutTransientCodec::REQUEST_TYPE = HZ_MAP_PUTTRANSIENT;
                const bool MapPutTransientCodec::RETRYABLE = false;
                const char* MapPutTransientCodec::OPERATION_NAME = "Map.PutTransient";
                const ResponseMessageConst MapPutTransientCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapPutTransientCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, ttl);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapPutTransientCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutTransientCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapPutWithMaxIdleCodec::REQUEST_TYPE = HZ_MAP_PUTWITHMAXIDLE;
                const bool MapPutWithMaxIdleCodec::RETRYABLE = false;
                const char* MapPutWithMaxIdleCodec::OPERATION_NAME = "Map.PutWithMaxIdle";
                const ResponseMessageConst MapPutWithMaxIdleCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapPutWithMaxIdleCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t maxIdle) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, ttl, maxIdle);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapPutWithMaxIdleCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->set(maxIdle);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapPutWithMaxIdleCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t maxIdle) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    dataSize += ClientMessage::calculateDataSize(maxIdle);
                    return dataSize;
                }

                MapPutWithMaxIdleCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapPutWithMaxIdleCodec::ResponseParameters
                MapPutWithMaxIdleCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapPutWithMaxIdleCodec::ResponseParameters(clientMessage);
                }

                MapPutWithMaxIdleCodec::ResponseParameters::ResponseParameters(
                        const MapPutWithMaxIdleCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapRemoveAllCodec::REQUEST_TYPE = HZ_MAP_REMOVEALL;
                const bool MapRemoveAllCodec::RETRYABLE = false;
                const char* MapRemoveAllCodec::OPERATION_NAME = "Map.RemoveAll";
                const ResponseMessageConst MapRemoveAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapRemoveAllCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapRemoveAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapRemoveAllCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapRemoveCodec::REQUEST_TYPE = HZ_MAP_REMOVE;
                const bool MapRemoveCodec::RETRYABLE = false;
                const char* MapRemoveCodec::OPERATION_NAME = "Map.Remove";
                const ResponseMessageConst MapRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapRemoveCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapRemoveCodec::ResponseParameters
                MapRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapRemoveCodec::ResponseParameters(clientMessage);
                }

                MapRemoveCodec::ResponseParameters::ResponseParameters(const MapRemoveCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapRemoveEntryListenerCodec::REQUEST_TYPE = HZ_MAP_REMOVEENTRYLISTENER;
                const bool MapRemoveEntryListenerCodec::RETRYABLE = true;
                const char* MapRemoveEntryListenerCodec::OPERATION_NAME = "Map.RemoveEntryListener";
                const ResponseMessageConst MapRemoveEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapRemoveEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapRemoveEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapRemoveEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                MapRemoveEntryListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapRemoveEntryListenerCodec::ResponseParameters
                MapRemoveEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapRemoveEntryListenerCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapRemoveIfSameCodec::REQUEST_TYPE = HZ_MAP_REMOVEIFSAME;
                const bool MapRemoveIfSameCodec::RETRYABLE = false;
                const char* MapRemoveIfSameCodec::OPERATION_NAME = "Map.RemoveIfSame";
                const ResponseMessageConst MapRemoveIfSameCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapRemoveIfSameCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapRemoveIfSameCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapRemoveIfSameCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapRemoveIfSameCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapRemoveIfSameCodec::ResponseParameters
                MapRemoveIfSameCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapRemoveIfSameCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapRemoveInterceptorCodec::REQUEST_TYPE = HZ_MAP_REMOVEINTERCEPTOR;
                const bool MapRemoveInterceptorCodec::RETRYABLE = false;
                const char* MapRemoveInterceptorCodec::OPERATION_NAME = "Map.RemoveInterceptor";
                const ResponseMessageConst MapRemoveInterceptorCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapRemoveInterceptorCodec::encodeRequest(
                        const std::string &name,
                        const std::string &id) {
                    int32_t requiredDataSize = calculateDataSize(name, id);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapRemoveInterceptorCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(id);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapRemoveInterceptorCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &id) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(id);
                    return dataSize;
                }

                MapRemoveInterceptorCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapRemoveInterceptorCodec::ResponseParameters
                MapRemoveInterceptorCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapRemoveInterceptorCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapReplaceCodec::REQUEST_TYPE = HZ_MAP_REPLACE;
                const bool MapReplaceCodec::RETRYABLE = false;
                const char* MapReplaceCodec::OPERATION_NAME = "Map.Replace";
                const ResponseMessageConst MapReplaceCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapReplaceCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapReplaceCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapReplaceCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapReplaceCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapReplaceCodec::ResponseParameters
                MapReplaceCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapReplaceCodec::ResponseParameters(clientMessage);
                }

                MapReplaceCodec::ResponseParameters::ResponseParameters(
                        const MapReplaceCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapReplaceIfSameCodec::REQUEST_TYPE = HZ_MAP_REPLACEIFSAME;
                const bool MapReplaceIfSameCodec::RETRYABLE = false;
                const char* MapReplaceIfSameCodec::OPERATION_NAME = "Map.ReplaceIfSame";
                const ResponseMessageConst MapReplaceIfSameCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapReplaceIfSameCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &testValue,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, testValue, value, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapReplaceIfSameCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(testValue);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapReplaceIfSameCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &testValue,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(testValue);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapReplaceIfSameCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapReplaceIfSameCodec::ResponseParameters
                MapReplaceIfSameCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapReplaceIfSameCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapSetCodec::REQUEST_TYPE = HZ_MAP_SET;
                const bool MapSetCodec::RETRYABLE = false;
                const char* MapSetCodec::OPERATION_NAME = "Map.Set";
                const ResponseMessageConst MapSetCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapSetCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, ttl);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapSetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapSetCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapSetWithMaxIdleCodec::REQUEST_TYPE = HZ_MAP_SETWITHMAXIDLE;
                const bool MapSetWithMaxIdleCodec::RETRYABLE = false;
                const char* MapSetWithMaxIdleCodec::OPERATION_NAME = "Map.SetWithMaxIdle";
                const ResponseMessageConst MapSetWithMaxIdleCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapSetWithMaxIdleCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t maxIdle) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, ttl, maxIdle);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapSetWithMaxIdleCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->set(maxIdle);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapSetWithMaxIdleCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t maxIdle) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    dataSize += ClientMessage::calculateDataSize(maxIdle);
                    return dataSize;
                }

                MapSetWithMaxIdleCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapSetWithMaxIdleCodec::ResponseParameters
                MapSetWithMaxIdleCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapSetWithMaxIdleCodec::ResponseParameters(clientMessage);
                }

                MapSetWithMaxIdleCodec::ResponseParameters::ResponseParameters(
                        const MapSetWithMaxIdleCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapSizeCodec::REQUEST_TYPE = HZ_MAP_SIZE;
                const bool MapSizeCodec::RETRYABLE = true;
                const char* MapSizeCodec::OPERATION_NAME = "Map.Size";
                const ResponseMessageConst MapSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> MapSizeCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapSizeCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MapSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                MapSizeCodec::ResponseParameters
                MapSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapSubmitToKeyCodec::REQUEST_TYPE = HZ_MAP_SUBMITTOKEY;
                const bool MapSubmitToKeyCodec::RETRYABLE = false;
                const char* MapSubmitToKeyCodec::OPERATION_NAME = "Map.SubmitToKey";
                const ResponseMessageConst MapSubmitToKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> MapSubmitToKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, entryProcessor, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapSubmitToKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(entryProcessor);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapSubmitToKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &entryProcessor,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(entryProcessor);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MapSubmitToKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                MapSubmitToKeyCodec::ResponseParameters
                MapSubmitToKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapSubmitToKeyCodec::ResponseParameters(clientMessage);
                }

                MapSubmitToKeyCodec::ResponseParameters::ResponseParameters(
                        const MapSubmitToKeyCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapTryLockCodec::REQUEST_TYPE = HZ_MAP_TRYLOCK;
                const bool MapTryLockCodec::RETRYABLE = true;
                const char* MapTryLockCodec::OPERATION_NAME = "Map.TryLock";
                const ResponseMessageConst MapTryLockCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapTryLockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, lease, timeout, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapTryLockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(lease);
                    clientMessage->set(timeout);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapTryLockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(lease);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }

                MapTryLockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapTryLockCodec::ResponseParameters
                MapTryLockCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapTryLockCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapTryPutCodec::REQUEST_TYPE = HZ_MAP_TRYPUT;
                const bool MapTryPutCodec::RETRYABLE = false;
                const char* MapTryPutCodec::OPERATION_NAME = "Map.TryPut";
                const ResponseMessageConst MapTryPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapTryPutCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId, timeout);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapTryPutCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapTryPutCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    return dataSize;
                }

                MapTryPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapTryPutCodec::ResponseParameters
                MapTryPutCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapTryPutCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapTryRemoveCodec::REQUEST_TYPE = HZ_MAP_TRYREMOVE;
                const bool MapTryRemoveCodec::RETRYABLE = false;
                const char* MapTryRemoveCodec::OPERATION_NAME = "Map.TryRemove";
                const ResponseMessageConst MapTryRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MapTryRemoveCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, timeout);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapTryRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapTryRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    return dataSize;
                }

                MapTryRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MapTryRemoveCodec::ResponseParameters
                MapTryRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapTryRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapUnlockCodec::REQUEST_TYPE = HZ_MAP_UNLOCK;
                const bool MapUnlockCodec::RETRYABLE = true;
                const char* MapUnlockCodec::OPERATION_NAME = "Map.Unlock";
                const ResponseMessageConst MapUnlockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MapUnlockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapUnlockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapUnlockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapValuesCodec::REQUEST_TYPE = HZ_MAP_VALUES;
                const bool MapValuesCodec::RETRYABLE = true;
                const char* MapValuesCodec::OPERATION_NAME = "Map.Values";
                const ResponseMessageConst MapValuesCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MapValuesCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapValuesCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapValuesCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MapValuesCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MapValuesCodec::ResponseParameters
                MapValuesCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapValuesCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapValuesWithPagingPredicateCodec::REQUEST_TYPE = HZ_MAP_VALUESWITHPAGINGPREDICATE;
                const bool MapValuesWithPagingPredicateCodec::RETRYABLE = true;
                const char* MapValuesWithPagingPredicateCodec::OPERATION_NAME = "Map.ValuesWithPagingPredicate";
                const ResponseMessageConst MapValuesWithPagingPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MapValuesWithPagingPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapValuesWithPagingPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapValuesWithPagingPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapValuesWithPagingPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MapValuesWithPagingPredicateCodec::ResponseParameters
                MapValuesWithPagingPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapValuesWithPagingPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MapMessageType MapValuesWithPredicateCodec::REQUEST_TYPE = HZ_MAP_VALUESWITHPREDICATE;
                const bool MapValuesWithPredicateCodec::RETRYABLE = true;
                const char* MapValuesWithPredicateCodec::OPERATION_NAME = "Map.ValuesWithPredicate";
                const ResponseMessageConst MapValuesWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MapValuesWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MapValuesWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MapValuesWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                MapValuesWithPredicateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MapValuesWithPredicateCodec::ResponseParameters
                MapValuesWithPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MapValuesWithPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapAddEntryListenerCodec::REQUEST_TYPE = HZ_MULTIMAP_ADDENTRYLISTENER;
                const bool MultiMapAddEntryListenerCodec::RETRYABLE = false;
                const char* MultiMapAddEntryListenerCodec::OPERATION_NAME = "MultiMap.AddEntryListener";
                const ResponseMessageConst MultiMapAddEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> MultiMapAddEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapAddEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapAddEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MultiMapAddEntryListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MultiMapAddEntryListenerCodec::ResponseParameters
                MultiMapAddEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapAddEntryListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                MultiMapAddEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MultiMapAddEntryListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[MultiMapAddEntryListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapAddEntryListenerToKeyCodec::REQUEST_TYPE = HZ_MULTIMAP_ADDENTRYLISTENERTOKEY;
                const bool MultiMapAddEntryListenerToKeyCodec::RETRYABLE = false;
                const char* MultiMapAddEntryListenerToKeyCodec::OPERATION_NAME = "MultiMap.AddEntryListenerToKey";
                const ResponseMessageConst MultiMapAddEntryListenerToKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> MultiMapAddEntryListenerToKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool includeValue,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, key, includeValue, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapAddEntryListenerToKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapAddEntryListenerToKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool includeValue,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                MultiMapAddEntryListenerToKeyCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                MultiMapAddEntryListenerToKeyCodec::ResponseParameters
                MultiMapAddEntryListenerToKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapAddEntryListenerToKeyCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                MultiMapAddEntryListenerToKeyCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void MultiMapAddEntryListenerToKeyCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[MultiMapAddEntryListenerToKeyCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapClearCodec::REQUEST_TYPE = HZ_MULTIMAP_CLEAR;
                const bool MultiMapClearCodec::RETRYABLE = false;
                const char* MultiMapClearCodec::OPERATION_NAME = "MultiMap.Clear";
                const ResponseMessageConst MultiMapClearCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MultiMapClearCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapClearCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapClearCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapContainsEntryCodec::REQUEST_TYPE = HZ_MULTIMAP_CONTAINSENTRY;
                const bool MultiMapContainsEntryCodec::RETRYABLE = true;
                const char* MultiMapContainsEntryCodec::OPERATION_NAME = "MultiMap.ContainsEntry";
                const ResponseMessageConst MultiMapContainsEntryCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapContainsEntryCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapContainsEntryCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapContainsEntryCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapContainsEntryCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapContainsEntryCodec::ResponseParameters
                MultiMapContainsEntryCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapContainsEntryCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapContainsKeyCodec::REQUEST_TYPE = HZ_MULTIMAP_CONTAINSKEY;
                const bool MultiMapContainsKeyCodec::RETRYABLE = true;
                const char* MultiMapContainsKeyCodec::OPERATION_NAME = "MultiMap.ContainsKey";
                const ResponseMessageConst MultiMapContainsKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapContainsKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapContainsKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapContainsKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapContainsKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapContainsKeyCodec::ResponseParameters
                MultiMapContainsKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapContainsKeyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapContainsValueCodec::REQUEST_TYPE = HZ_MULTIMAP_CONTAINSVALUE;
                const bool MultiMapContainsValueCodec::RETRYABLE = true;
                const char* MultiMapContainsValueCodec::OPERATION_NAME = "MultiMap.ContainsValue";
                const ResponseMessageConst MultiMapContainsValueCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapContainsValueCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapContainsValueCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapContainsValueCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                MultiMapContainsValueCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapContainsValueCodec::ResponseParameters
                MultiMapContainsValueCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapContainsValueCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapEntrySetCodec::REQUEST_TYPE = HZ_MULTIMAP_ENTRYSET;
                const bool MultiMapEntrySetCodec::RETRYABLE = true;
                const char* MultiMapEntrySetCodec::OPERATION_NAME = "MultiMap.EntrySet";
                const ResponseMessageConst MultiMapEntrySetCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> MultiMapEntrySetCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapEntrySetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapEntrySetCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MultiMapEntrySetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                MultiMapEntrySetCodec::ResponseParameters
                MultiMapEntrySetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapEntrySetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapForceUnlockCodec::REQUEST_TYPE = HZ_MULTIMAP_FORCEUNLOCK;
                const bool MultiMapForceUnlockCodec::RETRYABLE = true;
                const char* MultiMapForceUnlockCodec::OPERATION_NAME = "MultiMap.ForceUnlock";
                const ResponseMessageConst MultiMapForceUnlockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MultiMapForceUnlockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapForceUnlockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapForceUnlockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapGetCodec::REQUEST_TYPE = HZ_MULTIMAP_GET;
                const bool MultiMapGetCodec::RETRYABLE = true;
                const char* MultiMapGetCodec::OPERATION_NAME = "MultiMap.Get";
                const ResponseMessageConst MultiMapGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MultiMapGetCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapGetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapGetCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MultiMapGetCodec::ResponseParameters
                MultiMapGetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapGetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapIsLockedCodec::REQUEST_TYPE = HZ_MULTIMAP_ISLOCKED;
                const bool MultiMapIsLockedCodec::RETRYABLE = true;
                const char* MultiMapIsLockedCodec::OPERATION_NAME = "MultiMap.IsLocked";
                const ResponseMessageConst MultiMapIsLockedCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapIsLockedCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapIsLockedCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapIsLockedCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                MultiMapIsLockedCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapIsLockedCodec::ResponseParameters
                MultiMapIsLockedCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapIsLockedCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapKeySetCodec::REQUEST_TYPE = HZ_MULTIMAP_KEYSET;
                const bool MultiMapKeySetCodec::RETRYABLE = true;
                const char* MultiMapKeySetCodec::OPERATION_NAME = "MultiMap.KeySet";
                const ResponseMessageConst MultiMapKeySetCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MultiMapKeySetCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapKeySetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapKeySetCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MultiMapKeySetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MultiMapKeySetCodec::ResponseParameters
                MultiMapKeySetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapKeySetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapLockCodec::REQUEST_TYPE = HZ_MULTIMAP_LOCK;
                const bool MultiMapLockCodec::RETRYABLE = true;
                const char* MultiMapLockCodec::OPERATION_NAME = "MultiMap.Lock";
                const ResponseMessageConst MultiMapLockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MultiMapLockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, ttl, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapLockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(ttl);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapLockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t ttl,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapPutCodec::REQUEST_TYPE = HZ_MULTIMAP_PUT;
                const bool MultiMapPutCodec::RETRYABLE = false;
                const char* MultiMapPutCodec::OPERATION_NAME = "MultiMap.Put";
                const ResponseMessageConst MultiMapPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapPutCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapPutCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapPutCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapPutCodec::ResponseParameters
                MultiMapPutCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapPutCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapRemoveCodec::REQUEST_TYPE = HZ_MULTIMAP_REMOVE;
                const bool MultiMapRemoveCodec::RETRYABLE = false;
                const char* MultiMapRemoveCodec::OPERATION_NAME = "MultiMap.Remove";
                const ResponseMessageConst MultiMapRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MultiMapRemoveCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MultiMapRemoveCodec::ResponseParameters
                MultiMapRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapRemoveEntryCodec::REQUEST_TYPE = HZ_MULTIMAP_REMOVEENTRY;
                const bool MultiMapRemoveEntryCodec::RETRYABLE = false;
                const char* MultiMapRemoveEntryCodec::OPERATION_NAME = "MultiMap.RemoveEntry";
                const ResponseMessageConst MultiMapRemoveEntryCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapRemoveEntryCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapRemoveEntryCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapRemoveEntryCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapRemoveEntryCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapRemoveEntryCodec::ResponseParameters
                MultiMapRemoveEntryCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapRemoveEntryCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapRemoveEntryListenerCodec::REQUEST_TYPE = HZ_MULTIMAP_REMOVEENTRYLISTENER;
                const bool MultiMapRemoveEntryListenerCodec::RETRYABLE = true;
                const char* MultiMapRemoveEntryListenerCodec::OPERATION_NAME = "MultiMap.RemoveEntryListener";
                const ResponseMessageConst MultiMapRemoveEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapRemoveEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapRemoveEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapRemoveEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                MultiMapRemoveEntryListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapRemoveEntryListenerCodec::ResponseParameters
                MultiMapRemoveEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapRemoveEntryListenerCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapSizeCodec::REQUEST_TYPE = HZ_MULTIMAP_SIZE;
                const bool MultiMapSizeCodec::RETRYABLE = true;
                const char* MultiMapSizeCodec::OPERATION_NAME = "MultiMap.Size";
                const ResponseMessageConst MultiMapSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> MultiMapSizeCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapSizeCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MultiMapSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                MultiMapSizeCodec::ResponseParameters
                MultiMapSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapTryLockCodec::REQUEST_TYPE = HZ_MULTIMAP_TRYLOCK;
                const bool MultiMapTryLockCodec::RETRYABLE = true;
                const char* MultiMapTryLockCodec::OPERATION_NAME = "MultiMap.TryLock";
                const ResponseMessageConst MultiMapTryLockCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> MultiMapTryLockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, lease, timeout, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapTryLockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(lease);
                    clientMessage->set(timeout);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapTryLockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t lease,
                        int64_t timeout,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(lease);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }

                MultiMapTryLockCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                MultiMapTryLockCodec::ResponseParameters
                MultiMapTryLockCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapTryLockCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapUnlockCodec::REQUEST_TYPE = HZ_MULTIMAP_UNLOCK;
                const bool MultiMapUnlockCodec::RETRYABLE = true;
                const char* MultiMapUnlockCodec::OPERATION_NAME = "MultiMap.Unlock";
                const ResponseMessageConst MultiMapUnlockCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> MultiMapUnlockCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t referenceId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId, referenceId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapUnlockCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->set(referenceId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapUnlockCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId,
                        int64_t referenceId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(referenceId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapValueCountCodec::REQUEST_TYPE = HZ_MULTIMAP_VALUECOUNT;
                const bool MultiMapValueCountCodec::RETRYABLE = true;
                const char* MultiMapValueCountCodec::OPERATION_NAME = "MultiMap.ValueCount";
                const ResponseMessageConst MultiMapValueCountCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> MultiMapValueCountCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, key, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapValueCountCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapValueCountCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                MultiMapValueCountCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                MultiMapValueCountCodec::ResponseParameters
                MultiMapValueCountCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapValueCountCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const MultiMapMessageType MultiMapValuesCodec::REQUEST_TYPE = HZ_MULTIMAP_VALUES;
                const bool MultiMapValuesCodec::RETRYABLE = true;
                const char* MultiMapValuesCodec::OPERATION_NAME = "MultiMap.Values";
                const ResponseMessageConst MultiMapValuesCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> MultiMapValuesCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) MultiMapValuesCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t MultiMapValuesCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                MultiMapValuesCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                MultiMapValuesCodec::ResponseParameters
                MultiMapValuesCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return MultiMapValuesCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const PNCounterMessageType PNCounterAddCodec::REQUEST_TYPE = HZ_PNCOUNTER_ADD;
                const bool PNCounterAddCodec::RETRYABLE = false;
                const char* PNCounterAddCodec::OPERATION_NAME = "PNCounter.Add";
                const ResponseMessageConst PNCounterAddCodec::RESPONSE_TYPE = (ResponseMessageConst) 127;

                std::unique_ptr<ClientMessage> PNCounterAddCodec::encodeRequest(
                        const std::string &name,
                        int64_t delta,
                        bool getBeforeUpdate,
                        const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                        const Address &targetReplica) {
                    int32_t requiredDataSize = calculateDataSize(name, delta, getBeforeUpdate, replicaTimestamps,
                                                                 targetReplica);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) PNCounterAddCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(delta);
                    clientMessage->set(getBeforeUpdate);
                    clientMessage->setArray<std::pair<std::string, int64_t> >(replicaTimestamps);
                    clientMessage->set(targetReplica);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t PNCounterAddCodec::calculateDataSize(
                        const std::string &name,
                        int64_t delta,
                        bool getBeforeUpdate,
                        const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                        const Address &targetReplica) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(delta);
                    dataSize += ClientMessage::calculateDataSize(getBeforeUpdate);
                    dataSize += ClientMessage::calculateDataSize<std::pair<std::string, int64_t> >(replicaTimestamps);
                    dataSize += ClientMessage::calculateDataSize(targetReplica);
                    return dataSize;
                }

                PNCounterAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    value = clientMessage.get<int64_t>();


                    replicaTimestamps = clientMessage.getArray<std::pair<std::string, int64_t> >();


                    replicaCount = clientMessage.get<int32_t>();

                }

                PNCounterAddCodec::ResponseParameters
                PNCounterAddCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return PNCounterAddCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const PNCounterMessageType PNCounterGetCodec::REQUEST_TYPE = HZ_PNCOUNTER_GET;
                const bool PNCounterGetCodec::RETRYABLE = true;
                const char* PNCounterGetCodec::OPERATION_NAME = "PNCounter.Get";
                const ResponseMessageConst PNCounterGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 127;

                std::unique_ptr<ClientMessage> PNCounterGetCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                        const Address &targetReplica) {
                    int32_t requiredDataSize = calculateDataSize(name, replicaTimestamps, targetReplica);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) PNCounterGetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<std::pair<std::string, int64_t> >(replicaTimestamps);
                    clientMessage->set(targetReplica);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t PNCounterGetCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<std::pair<std::string, int64_t> > &replicaTimestamps,
                        const Address &targetReplica) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<std::pair<std::string, int64_t> >(replicaTimestamps);
                    dataSize += ClientMessage::calculateDataSize(targetReplica);
                    return dataSize;
                }

                PNCounterGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    value = clientMessage.get<int64_t>();


                    replicaTimestamps = clientMessage.getArray<std::pair<std::string, int64_t> >();


                    replicaCount = clientMessage.get<int32_t>();

                }

                PNCounterGetCodec::ResponseParameters
                PNCounterGetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return PNCounterGetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const PNCounterMessageType PNCounterGetConfiguredReplicaCountCodec::REQUEST_TYPE = HZ_PNCOUNTER_GETCONFIGUREDREPLICACOUNT;
                const bool PNCounterGetConfiguredReplicaCountCodec::RETRYABLE = true;
                const char* PNCounterGetConfiguredReplicaCountCodec::OPERATION_NAME = "PNCounter.GetConfiguredReplicaCount";
                const ResponseMessageConst PNCounterGetConfiguredReplicaCountCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> PNCounterGetConfiguredReplicaCountCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) PNCounterGetConfiguredReplicaCountCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t PNCounterGetConfiguredReplicaCountCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                PNCounterGetConfiguredReplicaCountCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                PNCounterGetConfiguredReplicaCountCodec::ResponseParameters
                PNCounterGetConfiguredReplicaCountCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return PNCounterGetConfiguredReplicaCountCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueAddAllCodec::REQUEST_TYPE = HZ_QUEUE_ADDALL;
                const bool QueueAddAllCodec::RETRYABLE = false;
                const char* QueueAddAllCodec::OPERATION_NAME = "Queue.AddAll";
                const ResponseMessageConst QueueAddAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueAddAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t requiredDataSize = calculateDataSize(name, dataList);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueAddAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(dataList);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueAddAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(dataList);
                    return dataSize;
                }

                QueueAddAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueAddAllCodec::ResponseParameters
                QueueAddAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueAddAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueAddListenerCodec::REQUEST_TYPE = HZ_QUEUE_ADDLISTENER;
                const bool QueueAddListenerCodec::RETRYABLE = false;
                const char* QueueAddListenerCodec::OPERATION_NAME = "Queue.AddListener";
                const ResponseMessageConst QueueAddListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> QueueAddListenerCodec::encodeRequest(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueAddListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueAddListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                QueueAddListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                QueueAddListenerCodec::ResponseParameters
                QueueAddListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueAddListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                QueueAddListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void QueueAddListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ITEM: {
                            std::unique_ptr<serialization::pimpl::Data> item = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t eventType = clientMessage->get<int32_t>();


                            handleItemEventV10(item, uuid, eventType);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[QueueAddListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueClearCodec::REQUEST_TYPE = HZ_QUEUE_CLEAR;
                const bool QueueClearCodec::RETRYABLE = false;
                const char* QueueClearCodec::OPERATION_NAME = "Queue.Clear";
                const ResponseMessageConst QueueClearCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> QueueClearCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueClearCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueClearCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueCompareAndRemoveAllCodec::REQUEST_TYPE = HZ_QUEUE_COMPAREANDREMOVEALL;
                const bool QueueCompareAndRemoveAllCodec::RETRYABLE = false;
                const char* QueueCompareAndRemoveAllCodec::OPERATION_NAME = "Queue.CompareAndRemoveAll";
                const ResponseMessageConst QueueCompareAndRemoveAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueCompareAndRemoveAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t requiredDataSize = calculateDataSize(name, dataList);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueCompareAndRemoveAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(dataList);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueCompareAndRemoveAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(dataList);
                    return dataSize;
                }

                QueueCompareAndRemoveAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueCompareAndRemoveAllCodec::ResponseParameters
                QueueCompareAndRemoveAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueCompareAndRemoveAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueCompareAndRetainAllCodec::REQUEST_TYPE = HZ_QUEUE_COMPAREANDRETAINALL;
                const bool QueueCompareAndRetainAllCodec::RETRYABLE = false;
                const char* QueueCompareAndRetainAllCodec::OPERATION_NAME = "Queue.CompareAndRetainAll";
                const ResponseMessageConst QueueCompareAndRetainAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueCompareAndRetainAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t requiredDataSize = calculateDataSize(name, dataList);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueCompareAndRetainAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(dataList);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueCompareAndRetainAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(dataList);
                    return dataSize;
                }

                QueueCompareAndRetainAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueCompareAndRetainAllCodec::ResponseParameters
                QueueCompareAndRetainAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueCompareAndRetainAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueContainsAllCodec::REQUEST_TYPE = HZ_QUEUE_CONTAINSALL;
                const bool QueueContainsAllCodec::RETRYABLE = false;
                const char* QueueContainsAllCodec::OPERATION_NAME = "Queue.ContainsAll";
                const ResponseMessageConst QueueContainsAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueContainsAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t requiredDataSize = calculateDataSize(name, dataList);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueContainsAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(dataList);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueContainsAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &dataList) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(dataList);
                    return dataSize;
                }

                QueueContainsAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueContainsAllCodec::ResponseParameters
                QueueContainsAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueContainsAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueContainsCodec::REQUEST_TYPE = HZ_QUEUE_CONTAINS;
                const bool QueueContainsCodec::RETRYABLE = false;
                const char* QueueContainsCodec::OPERATION_NAME = "Queue.Contains";
                const ResponseMessageConst QueueContainsCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueContainsCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueContainsCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueContainsCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                QueueContainsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueContainsCodec::ResponseParameters
                QueueContainsCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueContainsCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueDrainToCodec::REQUEST_TYPE = HZ_QUEUE_DRAINTO;
                const bool QueueDrainToCodec::RETRYABLE = false;
                const char* QueueDrainToCodec::OPERATION_NAME = "Queue.DrainTo";
                const ResponseMessageConst QueueDrainToCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> QueueDrainToCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueDrainToCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueDrainToCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueueDrainToCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                QueueDrainToCodec::ResponseParameters
                QueueDrainToCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueDrainToCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueDrainToMaxSizeCodec::REQUEST_TYPE = HZ_QUEUE_DRAINTOMAXSIZE;
                const bool QueueDrainToMaxSizeCodec::RETRYABLE = false;
                const char* QueueDrainToMaxSizeCodec::OPERATION_NAME = "Queue.DrainToMaxSize";
                const ResponseMessageConst QueueDrainToMaxSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> QueueDrainToMaxSizeCodec::encodeRequest(
                        const std::string &name,
                        int32_t maxSize) {
                    int32_t requiredDataSize = calculateDataSize(name, maxSize);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueDrainToMaxSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(maxSize);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueDrainToMaxSizeCodec::calculateDataSize(
                        const std::string &name,
                        int32_t maxSize) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(maxSize);
                    return dataSize;
                }

                QueueDrainToMaxSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                QueueDrainToMaxSizeCodec::ResponseParameters
                QueueDrainToMaxSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueDrainToMaxSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueIsEmptyCodec::REQUEST_TYPE = HZ_QUEUE_ISEMPTY;
                const bool QueueIsEmptyCodec::RETRYABLE = false;
                const char* QueueIsEmptyCodec::OPERATION_NAME = "Queue.IsEmpty";
                const ResponseMessageConst QueueIsEmptyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueIsEmptyCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueIsEmptyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueIsEmptyCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueueIsEmptyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueIsEmptyCodec::ResponseParameters
                QueueIsEmptyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueIsEmptyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueIteratorCodec::REQUEST_TYPE = HZ_QUEUE_ITERATOR;
                const bool QueueIteratorCodec::RETRYABLE = false;
                const char* QueueIteratorCodec::OPERATION_NAME = "Queue.Iterator";
                const ResponseMessageConst QueueIteratorCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> QueueIteratorCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueIteratorCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueIteratorCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueueIteratorCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                QueueIteratorCodec::ResponseParameters
                QueueIteratorCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueIteratorCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueOfferCodec::REQUEST_TYPE = HZ_QUEUE_OFFER;
                const bool QueueOfferCodec::RETRYABLE = false;
                const char* QueueOfferCodec::OPERATION_NAME = "Queue.Offer";
                const ResponseMessageConst QueueOfferCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueOfferCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value,
                        int64_t timeoutMillis) {
                    int32_t requiredDataSize = calculateDataSize(name, value, timeoutMillis);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueOfferCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->set(timeoutMillis);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueOfferCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value,
                        int64_t timeoutMillis) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(timeoutMillis);
                    return dataSize;
                }

                QueueOfferCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueOfferCodec::ResponseParameters
                QueueOfferCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueOfferCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueuePeekCodec::REQUEST_TYPE = HZ_QUEUE_PEEK;
                const bool QueuePeekCodec::RETRYABLE = false;
                const char* QueuePeekCodec::OPERATION_NAME = "Queue.Peek";
                const ResponseMessageConst QueuePeekCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> QueuePeekCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueuePeekCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueuePeekCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueuePeekCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                QueuePeekCodec::ResponseParameters
                QueuePeekCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueuePeekCodec::ResponseParameters(clientMessage);
                }

                QueuePeekCodec::ResponseParameters::ResponseParameters(const QueuePeekCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueuePollCodec::REQUEST_TYPE = HZ_QUEUE_POLL;
                const bool QueuePollCodec::RETRYABLE = false;
                const char* QueuePollCodec::OPERATION_NAME = "Queue.Poll";
                const ResponseMessageConst QueuePollCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> QueuePollCodec::encodeRequest(
                        const std::string &name,
                        int64_t timeoutMillis) {
                    int32_t requiredDataSize = calculateDataSize(name, timeoutMillis);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueuePollCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(timeoutMillis);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueuePollCodec::calculateDataSize(
                        const std::string &name,
                        int64_t timeoutMillis) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(timeoutMillis);
                    return dataSize;
                }

                QueuePollCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                QueuePollCodec::ResponseParameters
                QueuePollCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueuePollCodec::ResponseParameters(clientMessage);
                }

                QueuePollCodec::ResponseParameters::ResponseParameters(const QueuePollCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueuePutCodec::REQUEST_TYPE = HZ_QUEUE_PUT;
                const bool QueuePutCodec::RETRYABLE = false;
                const char* QueuePutCodec::OPERATION_NAME = "Queue.Put";
                const ResponseMessageConst QueuePutCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> QueuePutCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueuePutCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueuePutCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueRemainingCapacityCodec::REQUEST_TYPE = HZ_QUEUE_REMAININGCAPACITY;
                const bool QueueRemainingCapacityCodec::RETRYABLE = false;
                const char* QueueRemainingCapacityCodec::OPERATION_NAME = "Queue.RemainingCapacity";
                const ResponseMessageConst QueueRemainingCapacityCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> QueueRemainingCapacityCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueRemainingCapacityCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueRemainingCapacityCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueueRemainingCapacityCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                QueueRemainingCapacityCodec::ResponseParameters
                QueueRemainingCapacityCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueRemainingCapacityCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueRemoveCodec::REQUEST_TYPE = HZ_QUEUE_REMOVE;
                const bool QueueRemoveCodec::RETRYABLE = false;
                const char* QueueRemoveCodec::OPERATION_NAME = "Queue.Remove";
                const ResponseMessageConst QueueRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueRemoveCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                QueueRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueRemoveCodec::ResponseParameters
                QueueRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueRemoveListenerCodec::REQUEST_TYPE = HZ_QUEUE_REMOVELISTENER;
                const bool QueueRemoveListenerCodec::RETRYABLE = true;
                const char* QueueRemoveListenerCodec::OPERATION_NAME = "Queue.RemoveListener";
                const ResponseMessageConst QueueRemoveListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> QueueRemoveListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueRemoveListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueRemoveListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                QueueRemoveListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                QueueRemoveListenerCodec::ResponseParameters
                QueueRemoveListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueRemoveListenerCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const QueueMessageType QueueSizeCodec::REQUEST_TYPE = HZ_QUEUE_SIZE;
                const bool QueueSizeCodec::RETRYABLE = false;
                const char* QueueSizeCodec::OPERATION_NAME = "Queue.Size";
                const ResponseMessageConst QueueSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> QueueSizeCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) QueueSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t QueueSizeCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                QueueSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                QueueSizeCodec::ResponseParameters
                QueueSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return QueueSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapAddEntryListenerCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ADDENTRYLISTENER;
                const bool ReplicatedMapAddEntryListenerCodec::RETRYABLE = false;
                const char* ReplicatedMapAddEntryListenerCodec::OPERATION_NAME = "ReplicatedMap.AddEntryListener";
                const ResponseMessageConst ReplicatedMapAddEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ReplicatedMapAddEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapAddEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapAddEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ReplicatedMapAddEntryListenerCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ReplicatedMapAddEntryListenerCodec::ResponseParameters
                ReplicatedMapAddEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapAddEntryListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ReplicatedMapAddEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ReplicatedMapAddEntryListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ReplicatedMapAddEntryListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapAddEntryListenerToKeyCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ADDENTRYLISTENERTOKEY;
                const bool ReplicatedMapAddEntryListenerToKeyCodec::RETRYABLE = false;
                const char* ReplicatedMapAddEntryListenerToKeyCodec::OPERATION_NAME = "ReplicatedMap.AddEntryListenerToKey";
                const ResponseMessageConst ReplicatedMapAddEntryListenerToKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ReplicatedMapAddEntryListenerToKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, key, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapAddEntryListenerToKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapAddEntryListenerToKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters
                ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapAddEntryListenerToKeyCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ReplicatedMapAddEntryListenerToKeyCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ReplicatedMapAddEntryListenerToKeyCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ReplicatedMapAddEntryListenerToKeyCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ADDENTRYLISTENERTOKEYWITHPREDICATE;
                const bool ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::RETRYABLE = false;
                const char* ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::OPERATION_NAME = "ReplicatedMap.AddEntryListenerToKeyWithPredicate";
                const ResponseMessageConst ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &predicate,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, key, predicate, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(
                            (uint16_t) ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(predicate);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &predicate,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::ResponseParameters
                ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::ResponseParameters::decode(
                        ClientMessage clientMessage) {
                    return ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ReplicatedMapAddEntryListenerToKeyWithPredicateCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapAddEntryListenerWithPredicateCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ADDENTRYLISTENERWITHPREDICATE;
                const bool ReplicatedMapAddEntryListenerWithPredicateCodec::RETRYABLE = false;
                const char* ReplicatedMapAddEntryListenerWithPredicateCodec::OPERATION_NAME = "ReplicatedMap.AddEntryListenerWithPredicate";
                const ResponseMessageConst ReplicatedMapAddEntryListenerWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ReplicatedMapAddEntryListenerWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, predicate, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType(
                            (uint16_t) ReplicatedMapAddEntryListenerWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(predicate);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapAddEntryListenerWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &predicate,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ReplicatedMapAddEntryListenerWithPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ReplicatedMapAddEntryListenerWithPredicateCodec::ResponseParameters
                ReplicatedMapAddEntryListenerWithPredicateCodec::ResponseParameters::decode(
                        ClientMessage clientMessage) {
                    return ReplicatedMapAddEntryListenerWithPredicateCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ReplicatedMapAddEntryListenerWithPredicateCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ReplicatedMapAddEntryListenerWithPredicateCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ReplicatedMapAddEntryListenerWithPredicateCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapAddNearCacheEntryListenerCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ADDNEARCACHEENTRYLISTENER;
                const bool ReplicatedMapAddNearCacheEntryListenerCodec::RETRYABLE = false;
                const char* ReplicatedMapAddNearCacheEntryListenerCodec::OPERATION_NAME = "ReplicatedMap.AddNearCacheEntryListener";
                const ResponseMessageConst ReplicatedMapAddNearCacheEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> ReplicatedMapAddNearCacheEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapAddNearCacheEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapAddNearCacheEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters
                ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapAddNearCacheEntryListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ENTRY: {
                            std::unique_ptr<serialization::pimpl::Data> key = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> value = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> oldValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::unique_ptr<serialization::pimpl::Data> mergingValue = clientMessage->getNullable<serialization::pimpl::Data>();

                            int32_t eventType = clientMessage->get<int32_t>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t numberOfAffectedEntries = clientMessage->get<int32_t>();


                            handleEntryEventV10(key, value, oldValue, mergingValue, eventType, uuid,
                                                numberOfAffectedEntries);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[ReplicatedMapAddNearCacheEntryListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapClearCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_CLEAR;
                const bool ReplicatedMapClearCodec::RETRYABLE = false;
                const char* ReplicatedMapClearCodec::OPERATION_NAME = "ReplicatedMap.Clear";
                const ResponseMessageConst ReplicatedMapClearCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ReplicatedMapClearCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapClearCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapClearCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapContainsKeyCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_CONTAINSKEY;
                const bool ReplicatedMapContainsKeyCodec::RETRYABLE = true;
                const char* ReplicatedMapContainsKeyCodec::OPERATION_NAME = "ReplicatedMap.ContainsKey";
                const ResponseMessageConst ReplicatedMapContainsKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ReplicatedMapContainsKeyCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapContainsKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapContainsKeyCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                ReplicatedMapContainsKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ReplicatedMapContainsKeyCodec::ResponseParameters
                ReplicatedMapContainsKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapContainsKeyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapContainsValueCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_CONTAINSVALUE;
                const bool ReplicatedMapContainsValueCodec::RETRYABLE = true;
                const char* ReplicatedMapContainsValueCodec::OPERATION_NAME = "ReplicatedMap.ContainsValue";
                const ResponseMessageConst ReplicatedMapContainsValueCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ReplicatedMapContainsValueCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapContainsValueCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapContainsValueCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                ReplicatedMapContainsValueCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ReplicatedMapContainsValueCodec::ResponseParameters
                ReplicatedMapContainsValueCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapContainsValueCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapEntrySetCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ENTRYSET;
                const bool ReplicatedMapEntrySetCodec::RETRYABLE = true;
                const char* ReplicatedMapEntrySetCodec::OPERATION_NAME = "ReplicatedMap.EntrySet";
                const ResponseMessageConst ReplicatedMapEntrySetCodec::RESPONSE_TYPE = (ResponseMessageConst) 117;

                std::unique_ptr<ClientMessage> ReplicatedMapEntrySetCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapEntrySetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapEntrySetCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ReplicatedMapEntrySetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >();

                }

                ReplicatedMapEntrySetCodec::ResponseParameters
                ReplicatedMapEntrySetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapEntrySetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapGetCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_GET;
                const bool ReplicatedMapGetCodec::RETRYABLE = true;
                const char* ReplicatedMapGetCodec::OPERATION_NAME = "ReplicatedMap.Get";
                const ResponseMessageConst ReplicatedMapGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ReplicatedMapGetCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapGetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapGetCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                ReplicatedMapGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ReplicatedMapGetCodec::ResponseParameters
                ReplicatedMapGetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapGetCodec::ResponseParameters(clientMessage);
                }

                ReplicatedMapGetCodec::ResponseParameters::ResponseParameters(
                        const ReplicatedMapGetCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapIsEmptyCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_ISEMPTY;
                const bool ReplicatedMapIsEmptyCodec::RETRYABLE = true;
                const char* ReplicatedMapIsEmptyCodec::OPERATION_NAME = "ReplicatedMap.IsEmpty";
                const ResponseMessageConst ReplicatedMapIsEmptyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ReplicatedMapIsEmptyCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapIsEmptyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapIsEmptyCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ReplicatedMapIsEmptyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ReplicatedMapIsEmptyCodec::ResponseParameters
                ReplicatedMapIsEmptyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapIsEmptyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapKeySetCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_KEYSET;
                const bool ReplicatedMapKeySetCodec::RETRYABLE = true;
                const char* ReplicatedMapKeySetCodec::OPERATION_NAME = "ReplicatedMap.KeySet";
                const ResponseMessageConst ReplicatedMapKeySetCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> ReplicatedMapKeySetCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapKeySetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapKeySetCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ReplicatedMapKeySetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                ReplicatedMapKeySetCodec::ResponseParameters
                ReplicatedMapKeySetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapKeySetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapPutAllCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_PUTALL;
                const bool ReplicatedMapPutAllCodec::RETRYABLE = false;
                const char* ReplicatedMapPutAllCodec::OPERATION_NAME = "ReplicatedMap.PutAll";
                const ResponseMessageConst ReplicatedMapPutAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> ReplicatedMapPutAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries) {
                    int32_t requiredDataSize = calculateDataSize(name, entries);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapPutAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >(
                            entries);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapPutAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> > &entries) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<std::pair<serialization::pimpl::Data, serialization::pimpl::Data> >(
                            entries);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapPutCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_PUT;
                const bool ReplicatedMapPutCodec::RETRYABLE = false;
                const char* ReplicatedMapPutCodec::OPERATION_NAME = "ReplicatedMap.Put";
                const ResponseMessageConst ReplicatedMapPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ReplicatedMapPutCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, key, value, ttl);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapPutCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapPutCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }

                ReplicatedMapPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ReplicatedMapPutCodec::ResponseParameters
                ReplicatedMapPutCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapPutCodec::ResponseParameters(clientMessage);
                }

                ReplicatedMapPutCodec::ResponseParameters::ResponseParameters(
                        const ReplicatedMapPutCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapRemoveCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_REMOVE;
                const bool ReplicatedMapRemoveCodec::RETRYABLE = false;
                const char* ReplicatedMapRemoveCodec::OPERATION_NAME = "ReplicatedMap.Remove";
                const ResponseMessageConst ReplicatedMapRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> ReplicatedMapRemoveCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                ReplicatedMapRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                ReplicatedMapRemoveCodec::ResponseParameters
                ReplicatedMapRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapRemoveCodec::ResponseParameters(clientMessage);
                }

                ReplicatedMapRemoveCodec::ResponseParameters::ResponseParameters(
                        const ReplicatedMapRemoveCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapRemoveEntryListenerCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_REMOVEENTRYLISTENER;
                const bool ReplicatedMapRemoveEntryListenerCodec::RETRYABLE = true;
                const char* ReplicatedMapRemoveEntryListenerCodec::OPERATION_NAME = "ReplicatedMap.RemoveEntryListener";
                const ResponseMessageConst ReplicatedMapRemoveEntryListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> ReplicatedMapRemoveEntryListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapRemoveEntryListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapRemoveEntryListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                ReplicatedMapRemoveEntryListenerCodec::ResponseParameters
                ReplicatedMapRemoveEntryListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapRemoveEntryListenerCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapSizeCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_SIZE;
                const bool ReplicatedMapSizeCodec::RETRYABLE = true;
                const char* ReplicatedMapSizeCodec::OPERATION_NAME = "ReplicatedMap.Size";
                const ResponseMessageConst ReplicatedMapSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> ReplicatedMapSizeCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapSizeCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ReplicatedMapSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                ReplicatedMapSizeCodec::ResponseParameters
                ReplicatedMapSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const ReplicatedMapMessageType ReplicatedMapValuesCodec::REQUEST_TYPE = HZ_REPLICATEDMAP_VALUES;
                const bool ReplicatedMapValuesCodec::RETRYABLE = true;
                const char* ReplicatedMapValuesCodec::OPERATION_NAME = "ReplicatedMap.Values";
                const ResponseMessageConst ReplicatedMapValuesCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> ReplicatedMapValuesCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) ReplicatedMapValuesCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t ReplicatedMapValuesCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                ReplicatedMapValuesCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                ReplicatedMapValuesCodec::ResponseParameters
                ReplicatedMapValuesCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return ReplicatedMapValuesCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferAddAllCodec::REQUEST_TYPE = HZ_RINGBUFFER_ADDALL;
                const bool RingbufferAddAllCodec::RETRYABLE = false;
                const char* RingbufferAddAllCodec::OPERATION_NAME = "Ringbuffer.AddAll";
                const ResponseMessageConst RingbufferAddAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferAddAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList,
                        int32_t overflowPolicy) {
                    int32_t requiredDataSize = calculateDataSize(name, valueList, overflowPolicy);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferAddAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(valueList);
                    clientMessage->set(overflowPolicy);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferAddAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList,
                        int32_t overflowPolicy) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(valueList);
                    dataSize += ClientMessage::calculateDataSize(overflowPolicy);
                    return dataSize;
                }

                RingbufferAddAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferAddAllCodec::ResponseParameters
                RingbufferAddAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferAddAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferAddCodec::REQUEST_TYPE = HZ_RINGBUFFER_ADD;
                const bool RingbufferAddCodec::RETRYABLE = false;
                const char* RingbufferAddCodec::OPERATION_NAME = "Ringbuffer.Add";
                const ResponseMessageConst RingbufferAddCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferAddCodec::encodeRequest(
                        const std::string &name,
                        int32_t overflowPolicy,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, overflowPolicy, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferAddCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(overflowPolicy);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferAddCodec::calculateDataSize(
                        const std::string &name,
                        int32_t overflowPolicy,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(overflowPolicy);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                RingbufferAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferAddCodec::ResponseParameters
                RingbufferAddCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferAddCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferCapacityCodec::REQUEST_TYPE = HZ_RINGBUFFER_CAPACITY;
                const bool RingbufferCapacityCodec::RETRYABLE = true;
                const char* RingbufferCapacityCodec::OPERATION_NAME = "Ringbuffer.Capacity";
                const ResponseMessageConst RingbufferCapacityCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferCapacityCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferCapacityCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferCapacityCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                RingbufferCapacityCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferCapacityCodec::ResponseParameters
                RingbufferCapacityCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferCapacityCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferHeadSequenceCodec::REQUEST_TYPE = HZ_RINGBUFFER_HEADSEQUENCE;
                const bool RingbufferHeadSequenceCodec::RETRYABLE = true;
                const char* RingbufferHeadSequenceCodec::OPERATION_NAME = "Ringbuffer.HeadSequence";
                const ResponseMessageConst RingbufferHeadSequenceCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferHeadSequenceCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferHeadSequenceCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferHeadSequenceCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                RingbufferHeadSequenceCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferHeadSequenceCodec::ResponseParameters
                RingbufferHeadSequenceCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferHeadSequenceCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferReadManyCodec::REQUEST_TYPE = HZ_RINGBUFFER_READMANY;
                const bool RingbufferReadManyCodec::RETRYABLE = true;
                const char* RingbufferReadManyCodec::OPERATION_NAME = "Ringbuffer.ReadMany";
                const ResponseMessageConst RingbufferReadManyCodec::RESPONSE_TYPE = (ResponseMessageConst) 115;

                std::unique_ptr<ClientMessage> RingbufferReadManyCodec::encodeRequest(
                        const std::string &name,
                        int64_t startSequence,
                        int32_t minCount,
                        int32_t maxCount,
                        const serialization::pimpl::Data *filter) {
                    int32_t requiredDataSize = calculateDataSize(name, startSequence, minCount, maxCount, filter);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferReadManyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(startSequence);
                    clientMessage->set(minCount);
                    clientMessage->set(maxCount);
                    clientMessage->set(filter);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferReadManyCodec::calculateDataSize(
                        const std::string &name,
                        int64_t startSequence,
                        int32_t minCount,
                        int32_t maxCount,
                        const serialization::pimpl::Data *filter) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(startSequence);
                    dataSize += ClientMessage::calculateDataSize(minCount);
                    dataSize += ClientMessage::calculateDataSize(maxCount);
                    dataSize += ClientMessage::calculateDataSize(filter);
                    return dataSize;
                }

                RingbufferReadManyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {
                    itemSeqsExist = false;
                    nextSeqExist = false;


                    readCount = clientMessage.get<int32_t>();


                    items = clientMessage.getArray<serialization::pimpl::Data>();
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    itemSeqs = clientMessage.getNullableArray<int64_t>();
                    itemSeqsExist = true;
                    if (clientMessage.isComplete()) {
                        return;
                    }

                    nextSeq = clientMessage.get<int64_t>();
                    nextSeqExist = true;
                }

                RingbufferReadManyCodec::ResponseParameters
                RingbufferReadManyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferReadManyCodec::ResponseParameters(clientMessage);
                }

                RingbufferReadManyCodec::ResponseParameters::ResponseParameters(
                        const RingbufferReadManyCodec::ResponseParameters &rhs) {
                    readCount = rhs.readCount;
                    items = rhs.items;
                    itemSeqs = std::unique_ptr<std::vector<int64_t> >(new std::vector<int64_t>(*rhs.itemSeqs));
                    nextSeq = rhs.nextSeq;
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferReadOneCodec::REQUEST_TYPE = HZ_RINGBUFFER_READONE;
                const bool RingbufferReadOneCodec::RETRYABLE = true;
                const char* RingbufferReadOneCodec::OPERATION_NAME = "Ringbuffer.ReadOne";
                const ResponseMessageConst RingbufferReadOneCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> RingbufferReadOneCodec::encodeRequest(
                        const std::string &name,
                        int64_t sequence) {
                    int32_t requiredDataSize = calculateDataSize(name, sequence);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferReadOneCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(sequence);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferReadOneCodec::calculateDataSize(
                        const std::string &name,
                        int64_t sequence) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(sequence);
                    return dataSize;
                }

                RingbufferReadOneCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                RingbufferReadOneCodec::ResponseParameters
                RingbufferReadOneCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferReadOneCodec::ResponseParameters(clientMessage);
                }

                RingbufferReadOneCodec::ResponseParameters::ResponseParameters(
                        const RingbufferReadOneCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferRemainingCapacityCodec::REQUEST_TYPE = HZ_RINGBUFFER_REMAININGCAPACITY;
                const bool RingbufferRemainingCapacityCodec::RETRYABLE = true;
                const char* RingbufferRemainingCapacityCodec::OPERATION_NAME = "Ringbuffer.RemainingCapacity";
                const ResponseMessageConst RingbufferRemainingCapacityCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferRemainingCapacityCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferRemainingCapacityCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferRemainingCapacityCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                RingbufferRemainingCapacityCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferRemainingCapacityCodec::ResponseParameters
                RingbufferRemainingCapacityCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferRemainingCapacityCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferSizeCodec::REQUEST_TYPE = HZ_RINGBUFFER_SIZE;
                const bool RingbufferSizeCodec::RETRYABLE = true;
                const char* RingbufferSizeCodec::OPERATION_NAME = "Ringbuffer.Size";
                const ResponseMessageConst RingbufferSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferSizeCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferSizeCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                RingbufferSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferSizeCodec::ResponseParameters
                RingbufferSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const RingbufferMessageType RingbufferTailSequenceCodec::REQUEST_TYPE = HZ_RINGBUFFER_TAILSEQUENCE;
                const bool RingbufferTailSequenceCodec::RETRYABLE = true;
                const char* RingbufferTailSequenceCodec::OPERATION_NAME = "Ringbuffer.TailSequence";
                const ResponseMessageConst RingbufferTailSequenceCodec::RESPONSE_TYPE = (ResponseMessageConst) 103;

                std::unique_ptr<ClientMessage> RingbufferTailSequenceCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) RingbufferTailSequenceCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t RingbufferTailSequenceCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                RingbufferTailSequenceCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int64_t>();

                }

                RingbufferTailSequenceCodec::ResponseParameters
                RingbufferTailSequenceCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return RingbufferTailSequenceCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetAddAllCodec::REQUEST_TYPE = HZ_SET_ADDALL;
                const bool SetAddAllCodec::RETRYABLE = false;
                const char* SetAddAllCodec::OPERATION_NAME = "Set.AddAll";
                const ResponseMessageConst SetAddAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetAddAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList) {
                    int32_t requiredDataSize = calculateDataSize(name, valueList);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetAddAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(valueList);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetAddAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &valueList) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(valueList);
                    return dataSize;
                }

                SetAddAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetAddAllCodec::ResponseParameters
                SetAddAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetAddAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetAddCodec::REQUEST_TYPE = HZ_SET_ADD;
                const bool SetAddCodec::RETRYABLE = false;
                const char* SetAddCodec::OPERATION_NAME = "Set.Add";
                const ResponseMessageConst SetAddCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetAddCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetAddCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetAddCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                SetAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetAddCodec::ResponseParameters SetAddCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetAddCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetAddListenerCodec::REQUEST_TYPE = HZ_SET_ADDLISTENER;
                const bool SetAddListenerCodec::RETRYABLE = false;
                const char* SetAddListenerCodec::OPERATION_NAME = "Set.AddListener";
                const ResponseMessageConst SetAddListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> SetAddListenerCodec::encodeRequest(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, includeValue, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetAddListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(includeValue);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetAddListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool includeValue,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(includeValue);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                SetAddListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                SetAddListenerCodec::ResponseParameters
                SetAddListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetAddListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                SetAddListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void SetAddListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_ITEM: {
                            std::unique_ptr<serialization::pimpl::Data> item = clientMessage->getNullable<serialization::pimpl::Data>();

                            std::string uuid = clientMessage->get<std::string>();

                            int32_t eventType = clientMessage->get<int32_t>();


                            handleItemEventV10(item, uuid, eventType);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[SetAddListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetClearCodec::REQUEST_TYPE = HZ_SET_CLEAR;
                const bool SetClearCodec::RETRYABLE = false;
                const char* SetClearCodec::OPERATION_NAME = "Set.Clear";
                const ResponseMessageConst SetClearCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> SetClearCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetClearCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetClearCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetCompareAndRemoveAllCodec::REQUEST_TYPE = HZ_SET_COMPAREANDREMOVEALL;
                const bool SetCompareAndRemoveAllCodec::RETRYABLE = false;
                const char* SetCompareAndRemoveAllCodec::OPERATION_NAME = "Set.CompareAndRemoveAll";
                const ResponseMessageConst SetCompareAndRemoveAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetCompareAndRemoveAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t requiredDataSize = calculateDataSize(name, values);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetCompareAndRemoveAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(values);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetCompareAndRemoveAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(values);
                    return dataSize;
                }

                SetCompareAndRemoveAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetCompareAndRemoveAllCodec::ResponseParameters
                SetCompareAndRemoveAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetCompareAndRemoveAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetCompareAndRetainAllCodec::REQUEST_TYPE = HZ_SET_COMPAREANDRETAINALL;
                const bool SetCompareAndRetainAllCodec::RETRYABLE = false;
                const char* SetCompareAndRetainAllCodec::OPERATION_NAME = "Set.CompareAndRetainAll";
                const ResponseMessageConst SetCompareAndRetainAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetCompareAndRetainAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t requiredDataSize = calculateDataSize(name, values);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetCompareAndRetainAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(values);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetCompareAndRetainAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &values) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(values);
                    return dataSize;
                }

                SetCompareAndRetainAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetCompareAndRetainAllCodec::ResponseParameters
                SetCompareAndRetainAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetCompareAndRetainAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetContainsAllCodec::REQUEST_TYPE = HZ_SET_CONTAINSALL;
                const bool SetContainsAllCodec::RETRYABLE = false;
                const char* SetContainsAllCodec::OPERATION_NAME = "Set.ContainsAll";
                const ResponseMessageConst SetContainsAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetContainsAllCodec::encodeRequest(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &items) {
                    int32_t requiredDataSize = calculateDataSize(name, items);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetContainsAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->setArray<serialization::pimpl::Data>(items);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetContainsAllCodec::calculateDataSize(
                        const std::string &name,
                        const std::vector<serialization::pimpl::Data> &items) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize<serialization::pimpl::Data>(items);
                    return dataSize;
                }

                SetContainsAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetContainsAllCodec::ResponseParameters
                SetContainsAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetContainsAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetContainsCodec::REQUEST_TYPE = HZ_SET_CONTAINS;
                const bool SetContainsCodec::RETRYABLE = false;
                const char* SetContainsCodec::OPERATION_NAME = "Set.Contains";
                const ResponseMessageConst SetContainsCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetContainsCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetContainsCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetContainsCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                SetContainsCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetContainsCodec::ResponseParameters
                SetContainsCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetContainsCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetGetAllCodec::REQUEST_TYPE = HZ_SET_GETALL;
                const bool SetGetAllCodec::RETRYABLE = false;
                const char* SetGetAllCodec::OPERATION_NAME = "Set.GetAll";
                const ResponseMessageConst SetGetAllCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> SetGetAllCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetGetAllCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetGetAllCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                SetGetAllCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                SetGetAllCodec::ResponseParameters
                SetGetAllCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetGetAllCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetIsEmptyCodec::REQUEST_TYPE = HZ_SET_ISEMPTY;
                const bool SetIsEmptyCodec::RETRYABLE = false;
                const char* SetIsEmptyCodec::OPERATION_NAME = "Set.IsEmpty";
                const ResponseMessageConst SetIsEmptyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetIsEmptyCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetIsEmptyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetIsEmptyCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                SetIsEmptyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetIsEmptyCodec::ResponseParameters
                SetIsEmptyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetIsEmptyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetRemoveCodec::REQUEST_TYPE = HZ_SET_REMOVE;
                const bool SetRemoveCodec::RETRYABLE = false;
                const char* SetRemoveCodec::OPERATION_NAME = "Set.Remove";
                const ResponseMessageConst SetRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetRemoveCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                SetRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetRemoveCodec::ResponseParameters
                SetRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetRemoveListenerCodec::REQUEST_TYPE = HZ_SET_REMOVELISTENER;
                const bool SetRemoveListenerCodec::RETRYABLE = true;
                const char* SetRemoveListenerCodec::OPERATION_NAME = "Set.RemoveListener";
                const ResponseMessageConst SetRemoveListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> SetRemoveListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetRemoveListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetRemoveListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                SetRemoveListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                SetRemoveListenerCodec::ResponseParameters
                SetRemoveListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetRemoveListenerCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const SetMessageType SetSizeCodec::REQUEST_TYPE = HZ_SET_SIZE;
                const bool SetSizeCodec::RETRYABLE = false;
                const char* SetSizeCodec::OPERATION_NAME = "Set.Size";
                const ResponseMessageConst SetSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> SetSizeCodec::encodeRequest(
                        const std::string &name) {
                    int32_t requiredDataSize = calculateDataSize(name);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) SetSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t SetSizeCodec::calculateDataSize(
                        const std::string &name) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    return dataSize;
                }

                SetSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                SetSizeCodec::ResponseParameters
                SetSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return SetSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TopicMessageType TopicAddMessageListenerCodec::REQUEST_TYPE = HZ_TOPIC_ADDMESSAGELISTENER;
                const bool TopicAddMessageListenerCodec::RETRYABLE = false;
                const char* TopicAddMessageListenerCodec::OPERATION_NAME = "Topic.AddMessageListener";
                const ResponseMessageConst TopicAddMessageListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> TopicAddMessageListenerCodec::encodeRequest(
                        const std::string &name,
                        bool localOnly) {
                    int32_t requiredDataSize = calculateDataSize(name, localOnly);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TopicAddMessageListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(localOnly);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TopicAddMessageListenerCodec::calculateDataSize(
                        const std::string &name,
                        bool localOnly) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(localOnly);
                    return dataSize;
                }

                TopicAddMessageListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                TopicAddMessageListenerCodec::ResponseParameters
                TopicAddMessageListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TopicAddMessageListenerCodec::ResponseParameters(clientMessage);
                }


                //************************ EVENTS START*************************************************************************//
                TopicAddMessageListenerCodec::AbstractEventHandler::~AbstractEventHandler() {
                }

                void TopicAddMessageListenerCodec::AbstractEventHandler::handle(
                        std::unique_ptr<protocol::ClientMessage> clientMessage) {
                    int messageType = clientMessage->getMessageType();
                    switch (messageType) {
                        case protocol::EVENT_TOPIC: {
                            serialization::pimpl::Data item = clientMessage->get<serialization::pimpl::Data>();

                            int64_t publishTime = clientMessage->get<int64_t>();

                            std::string uuid = clientMessage->get<std::string>();


                            handleTopicEventV10(item, publishTime, uuid);
                            break;
                        }
                        default:
                            getLogger()->warning(
                                    "[TopicAddMessageListenerCodec::AbstractEventHandler::handle] Unknown message type (",
                                    messageType, ") received on event handler.");
                    }
                }
                //************************ EVENTS END **************************************************************************//
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TopicMessageType TopicPublishCodec::REQUEST_TYPE = HZ_TOPIC_PUBLISH;
                const bool TopicPublishCodec::RETRYABLE = false;
                const char* TopicPublishCodec::OPERATION_NAME = "Topic.Publish";
                const ResponseMessageConst TopicPublishCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> TopicPublishCodec::encodeRequest(
                        const std::string &name,
                        const serialization::pimpl::Data &message) {
                    int32_t requiredDataSize = calculateDataSize(name, message);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TopicPublishCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(message);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TopicPublishCodec::calculateDataSize(
                        const std::string &name,
                        const serialization::pimpl::Data &message) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(message);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TopicMessageType TopicRemoveMessageListenerCodec::REQUEST_TYPE = HZ_TOPIC_REMOVEMESSAGELISTENER;
                const bool TopicRemoveMessageListenerCodec::RETRYABLE = true;
                const char* TopicRemoveMessageListenerCodec::OPERATION_NAME = "Topic.RemoveMessageListener";
                const ResponseMessageConst TopicRemoveMessageListenerCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TopicRemoveMessageListenerCodec::encodeRequest(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t requiredDataSize = calculateDataSize(name, registrationId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TopicRemoveMessageListenerCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(registrationId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TopicRemoveMessageListenerCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &registrationId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(registrationId);
                    return dataSize;
                }

                TopicRemoveMessageListenerCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TopicRemoveMessageListenerCodec::ResponseParameters
                TopicRemoveMessageListenerCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TopicRemoveMessageListenerCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionMessageType TransactionCommitCodec::REQUEST_TYPE = HZ_TRANSACTION_COMMIT;
                const bool TransactionCommitCodec::RETRYABLE = false;
                const char* TransactionCommitCodec::OPERATION_NAME = "Transaction.Commit";
                const ResponseMessageConst TransactionCommitCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> TransactionCommitCodec::encodeRequest(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(transactionId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionCommitCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(transactionId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionCommitCodec::calculateDataSize(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(transactionId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionMessageType TransactionCreateCodec::REQUEST_TYPE = HZ_TRANSACTION_CREATE;
                const bool TransactionCreateCodec::RETRYABLE = false;
                const char* TransactionCreateCodec::OPERATION_NAME = "Transaction.Create";
                const ResponseMessageConst TransactionCreateCodec::RESPONSE_TYPE = (ResponseMessageConst) 104;

                std::unique_ptr<ClientMessage> TransactionCreateCodec::encodeRequest(
                        int64_t timeout,
                        int32_t durability,
                        int32_t transactionType,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(timeout, durability, transactionType, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionCreateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(timeout);
                    clientMessage->set(durability);
                    clientMessage->set(transactionType);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionCreateCodec::calculateDataSize(
                        int64_t timeout,
                        int32_t durability,
                        int32_t transactionType,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    dataSize += ClientMessage::calculateDataSize(durability);
                    dataSize += ClientMessage::calculateDataSize(transactionType);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionCreateCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<std::string>();

                }

                TransactionCreateCodec::ResponseParameters
                TransactionCreateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionCreateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionMessageType TransactionRollbackCodec::REQUEST_TYPE = HZ_TRANSACTION_ROLLBACK;
                const bool TransactionRollbackCodec::RETRYABLE = false;
                const char* TransactionRollbackCodec::OPERATION_NAME = "Transaction.Rollback";
                const ResponseMessageConst TransactionRollbackCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> TransactionRollbackCodec::encodeRequest(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(transactionId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionRollbackCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(transactionId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionRollbackCodec::calculateDataSize(
                        const std::string &transactionId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(transactionId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalListMessageType TransactionalListAddCodec::REQUEST_TYPE = HZ_TRANSACTIONALLIST_ADD;
                const bool TransactionalListAddCodec::RETRYABLE = false;
                const char* TransactionalListAddCodec::OPERATION_NAME = "TransactionalList.Add";
                const ResponseMessageConst TransactionalListAddCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalListAddCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, item);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalListAddCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(item);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalListAddCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(item);
                    return dataSize;
                }

                TransactionalListAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalListAddCodec::ResponseParameters
                TransactionalListAddCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalListAddCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalListMessageType TransactionalListRemoveCodec::REQUEST_TYPE = HZ_TRANSACTIONALLIST_REMOVE;
                const bool TransactionalListRemoveCodec::RETRYABLE = false;
                const char* TransactionalListRemoveCodec::OPERATION_NAME = "TransactionalList.Remove";
                const ResponseMessageConst TransactionalListRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalListRemoveCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, item);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalListRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(item);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalListRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(item);
                    return dataSize;
                }

                TransactionalListRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalListRemoveCodec::ResponseParameters
                TransactionalListRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalListRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalListMessageType TransactionalListSizeCodec::REQUEST_TYPE = HZ_TRANSACTIONALLIST_SIZE;
                const bool TransactionalListSizeCodec::RETRYABLE = false;
                const char* TransactionalListSizeCodec::OPERATION_NAME = "TransactionalList.Size";
                const ResponseMessageConst TransactionalListSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> TransactionalListSizeCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalListSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalListSizeCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalListSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                TransactionalListSizeCodec::ResponseParameters
                TransactionalListSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalListSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapContainsKeyCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_CONTAINSKEY;
                const bool TransactionalMapContainsKeyCodec::RETRYABLE = false;
                const char* TransactionalMapContainsKeyCodec::OPERATION_NAME = "TransactionalMap.ContainsKey";
                const ResponseMessageConst TransactionalMapContainsKeyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalMapContainsKeyCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapContainsKeyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapContainsKeyCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                TransactionalMapContainsKeyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalMapContainsKeyCodec::ResponseParameters
                TransactionalMapContainsKeyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapContainsKeyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapDeleteCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_DELETE;
                const bool TransactionalMapDeleteCodec::RETRYABLE = false;
                const char* TransactionalMapDeleteCodec::OPERATION_NAME = "TransactionalMap.Delete";
                const ResponseMessageConst TransactionalMapDeleteCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> TransactionalMapDeleteCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapDeleteCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapDeleteCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapGetCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_GET;
                const bool TransactionalMapGetCodec::RETRYABLE = false;
                const char* TransactionalMapGetCodec::OPERATION_NAME = "TransactionalMap.Get";
                const ResponseMessageConst TransactionalMapGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> TransactionalMapGetCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapGetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapGetCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                TransactionalMapGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                TransactionalMapGetCodec::ResponseParameters
                TransactionalMapGetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapGetCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapGetCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapGetCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapIsEmptyCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_ISEMPTY;
                const bool TransactionalMapIsEmptyCodec::RETRYABLE = false;
                const char* TransactionalMapIsEmptyCodec::OPERATION_NAME = "TransactionalMap.IsEmpty";
                const ResponseMessageConst TransactionalMapIsEmptyCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalMapIsEmptyCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapIsEmptyCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapIsEmptyCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalMapIsEmptyCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalMapIsEmptyCodec::ResponseParameters
                TransactionalMapIsEmptyCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapIsEmptyCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapKeySetCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_KEYSET;
                const bool TransactionalMapKeySetCodec::RETRYABLE = false;
                const char* TransactionalMapKeySetCodec::OPERATION_NAME = "TransactionalMap.KeySet";
                const ResponseMessageConst TransactionalMapKeySetCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> TransactionalMapKeySetCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapKeySetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapKeySetCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalMapKeySetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                TransactionalMapKeySetCodec::ResponseParameters
                TransactionalMapKeySetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapKeySetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapKeySetWithPredicateCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_KEYSETWITHPREDICATE;
                const bool TransactionalMapKeySetWithPredicateCodec::RETRYABLE = false;
                const char* TransactionalMapKeySetWithPredicateCodec::OPERATION_NAME = "TransactionalMap.KeySetWithPredicate";
                const ResponseMessageConst TransactionalMapKeySetWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> TransactionalMapKeySetWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapKeySetWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapKeySetWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                TransactionalMapKeySetWithPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                TransactionalMapKeySetWithPredicateCodec::ResponseParameters
                TransactionalMapKeySetWithPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapKeySetWithPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapPutCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_PUT;
                const bool TransactionalMapPutCodec::RETRYABLE = false;
                const char* TransactionalMapPutCodec::OPERATION_NAME = "TransactionalMap.Put";
                const ResponseMessageConst TransactionalMapPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> TransactionalMapPutCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t ttl) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value, ttl);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapPutCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->set(ttl);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapPutCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value,
                        int64_t ttl) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    dataSize += ClientMessage::calculateDataSize(ttl);
                    return dataSize;
                }

                TransactionalMapPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                TransactionalMapPutCodec::ResponseParameters
                TransactionalMapPutCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapPutCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapPutCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapPutCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapPutIfAbsentCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_PUTIFABSENT;
                const bool TransactionalMapPutIfAbsentCodec::RETRYABLE = false;
                const char* TransactionalMapPutIfAbsentCodec::OPERATION_NAME = "TransactionalMap.PutIfAbsent";
                const ResponseMessageConst TransactionalMapPutIfAbsentCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> TransactionalMapPutIfAbsentCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapPutIfAbsentCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapPutIfAbsentCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                TransactionalMapPutIfAbsentCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                TransactionalMapPutIfAbsentCodec::ResponseParameters
                TransactionalMapPutIfAbsentCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapPutIfAbsentCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapPutIfAbsentCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapPutIfAbsentCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapRemoveCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_REMOVE;
                const bool TransactionalMapRemoveCodec::RETRYABLE = false;
                const char* TransactionalMapRemoveCodec::OPERATION_NAME = "TransactionalMap.Remove";
                const ResponseMessageConst TransactionalMapRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> TransactionalMapRemoveCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                TransactionalMapRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                TransactionalMapRemoveCodec::ResponseParameters
                TransactionalMapRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapRemoveCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapRemoveCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapRemoveCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapRemoveIfSameCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_REMOVEIFSAME;
                const bool TransactionalMapRemoveIfSameCodec::RETRYABLE = false;
                const char* TransactionalMapRemoveIfSameCodec::OPERATION_NAME = "TransactionalMap.RemoveIfSame";
                const ResponseMessageConst TransactionalMapRemoveIfSameCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalMapRemoveIfSameCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapRemoveIfSameCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapRemoveIfSameCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                TransactionalMapRemoveIfSameCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalMapRemoveIfSameCodec::ResponseParameters
                TransactionalMapRemoveIfSameCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapRemoveIfSameCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapReplaceCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_REPLACE;
                const bool TransactionalMapReplaceCodec::RETRYABLE = false;
                const char* TransactionalMapReplaceCodec::OPERATION_NAME = "TransactionalMap.Replace";
                const ResponseMessageConst TransactionalMapReplaceCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> TransactionalMapReplaceCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapReplaceCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapReplaceCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                TransactionalMapReplaceCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                TransactionalMapReplaceCodec::ResponseParameters
                TransactionalMapReplaceCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapReplaceCodec::ResponseParameters(clientMessage);
                }

                TransactionalMapReplaceCodec::ResponseParameters::ResponseParameters(
                        const TransactionalMapReplaceCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapReplaceIfSameCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_REPLACEIFSAME;
                const bool TransactionalMapReplaceIfSameCodec::RETRYABLE = false;
                const char* TransactionalMapReplaceIfSameCodec::OPERATION_NAME = "TransactionalMap.ReplaceIfSame";
                const ResponseMessageConst TransactionalMapReplaceIfSameCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalMapReplaceIfSameCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &oldValue,
                        const serialization::pimpl::Data &newValue) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, oldValue, newValue);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapReplaceIfSameCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(oldValue);
                    clientMessage->set(newValue);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapReplaceIfSameCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &oldValue,
                        const serialization::pimpl::Data &newValue) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(oldValue);
                    dataSize += ClientMessage::calculateDataSize(newValue);
                    return dataSize;
                }

                TransactionalMapReplaceIfSameCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalMapReplaceIfSameCodec::ResponseParameters
                TransactionalMapReplaceIfSameCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapReplaceIfSameCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapSetCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_SET;
                const bool TransactionalMapSetCodec::RETRYABLE = false;
                const char* TransactionalMapSetCodec::OPERATION_NAME = "TransactionalMap.Set";
                const ResponseMessageConst TransactionalMapSetCodec::RESPONSE_TYPE = (ResponseMessageConst) 100;

                std::unique_ptr<ClientMessage> TransactionalMapSetCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapSetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapSetCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapSizeCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_SIZE;
                const bool TransactionalMapSizeCodec::RETRYABLE = false;
                const char* TransactionalMapSizeCodec::OPERATION_NAME = "TransactionalMap.Size";
                const ResponseMessageConst TransactionalMapSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> TransactionalMapSizeCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapSizeCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalMapSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                TransactionalMapSizeCodec::ResponseParameters
                TransactionalMapSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapValuesCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_VALUES;
                const bool TransactionalMapValuesCodec::RETRYABLE = false;
                const char* TransactionalMapValuesCodec::OPERATION_NAME = "TransactionalMap.Values";
                const ResponseMessageConst TransactionalMapValuesCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> TransactionalMapValuesCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapValuesCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapValuesCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalMapValuesCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                TransactionalMapValuesCodec::ResponseParameters
                TransactionalMapValuesCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapValuesCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMapMessageType TransactionalMapValuesWithPredicateCodec::REQUEST_TYPE = HZ_TRANSACTIONALMAP_VALUESWITHPREDICATE;
                const bool TransactionalMapValuesWithPredicateCodec::RETRYABLE = false;
                const char* TransactionalMapValuesWithPredicateCodec::OPERATION_NAME = "TransactionalMap.ValuesWithPredicate";
                const ResponseMessageConst TransactionalMapValuesWithPredicateCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> TransactionalMapValuesWithPredicateCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &predicate) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, predicate);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMapValuesWithPredicateCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(predicate);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMapValuesWithPredicateCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &predicate) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(predicate);
                    return dataSize;
                }

                TransactionalMapValuesWithPredicateCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                TransactionalMapValuesWithPredicateCodec::ResponseParameters
                TransactionalMapValuesWithPredicateCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMapValuesWithPredicateCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMultiMapMessageType TransactionalMultiMapGetCodec::REQUEST_TYPE = HZ_TRANSACTIONALMULTIMAP_GET;
                const bool TransactionalMultiMapGetCodec::RETRYABLE = false;
                const char* TransactionalMultiMapGetCodec::OPERATION_NAME = "TransactionalMultiMap.Get";
                const ResponseMessageConst TransactionalMultiMapGetCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> TransactionalMultiMapGetCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMultiMapGetCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMultiMapGetCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                TransactionalMultiMapGetCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                TransactionalMultiMapGetCodec::ResponseParameters
                TransactionalMultiMapGetCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMultiMapGetCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMultiMapMessageType TransactionalMultiMapPutCodec::REQUEST_TYPE = HZ_TRANSACTIONALMULTIMAP_PUT;
                const bool TransactionalMultiMapPutCodec::RETRYABLE = false;
                const char* TransactionalMultiMapPutCodec::OPERATION_NAME = "TransactionalMultiMap.Put";
                const ResponseMessageConst TransactionalMultiMapPutCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalMultiMapPutCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMultiMapPutCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMultiMapPutCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                TransactionalMultiMapPutCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalMultiMapPutCodec::ResponseParameters
                TransactionalMultiMapPutCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMultiMapPutCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMultiMapMessageType TransactionalMultiMapRemoveCodec::REQUEST_TYPE = HZ_TRANSACTIONALMULTIMAP_REMOVE;
                const bool TransactionalMultiMapRemoveCodec::RETRYABLE = false;
                const char* TransactionalMultiMapRemoveCodec::OPERATION_NAME = "TransactionalMultiMap.Remove";
                const ResponseMessageConst TransactionalMultiMapRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 106;

                std::unique_ptr<ClientMessage> TransactionalMultiMapRemoveCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMultiMapRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMultiMapRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                TransactionalMultiMapRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getArray<serialization::pimpl::Data>();

                }

                TransactionalMultiMapRemoveCodec::ResponseParameters
                TransactionalMultiMapRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMultiMapRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMultiMapMessageType TransactionalMultiMapRemoveEntryCodec::REQUEST_TYPE = HZ_TRANSACTIONALMULTIMAP_REMOVEENTRY;
                const bool TransactionalMultiMapRemoveEntryCodec::RETRYABLE = false;
                const char* TransactionalMultiMapRemoveEntryCodec::OPERATION_NAME = "TransactionalMultiMap.RemoveEntry";
                const ResponseMessageConst TransactionalMultiMapRemoveEntryCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalMultiMapRemoveEntryCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key, value);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMultiMapRemoveEntryCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->set(value);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMultiMapRemoveEntryCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key,
                        const serialization::pimpl::Data &value) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    dataSize += ClientMessage::calculateDataSize(value);
                    return dataSize;
                }

                TransactionalMultiMapRemoveEntryCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalMultiMapRemoveEntryCodec::ResponseParameters
                TransactionalMultiMapRemoveEntryCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMultiMapRemoveEntryCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMultiMapMessageType TransactionalMultiMapSizeCodec::REQUEST_TYPE = HZ_TRANSACTIONALMULTIMAP_SIZE;
                const bool TransactionalMultiMapSizeCodec::RETRYABLE = false;
                const char* TransactionalMultiMapSizeCodec::OPERATION_NAME = "TransactionalMultiMap.Size";
                const ResponseMessageConst TransactionalMultiMapSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> TransactionalMultiMapSizeCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMultiMapSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMultiMapSizeCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalMultiMapSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                TransactionalMultiMapSizeCodec::ResponseParameters
                TransactionalMultiMapSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMultiMapSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalMultiMapMessageType TransactionalMultiMapValueCountCodec::REQUEST_TYPE = HZ_TRANSACTIONALMULTIMAP_VALUECOUNT;
                const bool TransactionalMultiMapValueCountCodec::RETRYABLE = false;
                const char* TransactionalMultiMapValueCountCodec::OPERATION_NAME = "TransactionalMultiMap.ValueCount";
                const ResponseMessageConst TransactionalMultiMapValueCountCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> TransactionalMultiMapValueCountCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, key);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalMultiMapValueCountCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(key);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalMultiMapValueCountCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &key) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(key);
                    return dataSize;
                }

                TransactionalMultiMapValueCountCodec::ResponseParameters::ResponseParameters(
                        ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                TransactionalMultiMapValueCountCodec::ResponseParameters
                TransactionalMultiMapValueCountCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalMultiMapValueCountCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalQueueMessageType TransactionalQueueOfferCodec::REQUEST_TYPE = HZ_TRANSACTIONALQUEUE_OFFER;
                const bool TransactionalQueueOfferCodec::RETRYABLE = false;
                const char* TransactionalQueueOfferCodec::OPERATION_NAME = "TransactionalQueue.Offer";
                const ResponseMessageConst TransactionalQueueOfferCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalQueueOfferCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item,
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, item, timeout);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalQueueOfferCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(item);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalQueueOfferCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item,
                        int64_t timeout) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(item);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    return dataSize;
                }

                TransactionalQueueOfferCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalQueueOfferCodec::ResponseParameters
                TransactionalQueueOfferCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalQueueOfferCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalQueueMessageType TransactionalQueuePollCodec::REQUEST_TYPE = HZ_TRANSACTIONALQUEUE_POLL;
                const bool TransactionalQueuePollCodec::RETRYABLE = false;
                const char* TransactionalQueuePollCodec::OPERATION_NAME = "TransactionalQueue.Poll";
                const ResponseMessageConst TransactionalQueuePollCodec::RESPONSE_TYPE = (ResponseMessageConst) 105;

                std::unique_ptr<ClientMessage> TransactionalQueuePollCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, timeout);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalQueuePollCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(timeout);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalQueuePollCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        int64_t timeout) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(timeout);
                    return dataSize;
                }

                TransactionalQueuePollCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.getNullable<serialization::pimpl::Data>();

                }

                TransactionalQueuePollCodec::ResponseParameters
                TransactionalQueuePollCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalQueuePollCodec::ResponseParameters(clientMessage);
                }

                TransactionalQueuePollCodec::ResponseParameters::ResponseParameters(
                        const TransactionalQueuePollCodec::ResponseParameters &rhs) {
                    response = std::unique_ptr<serialization::pimpl::Data>(
                            new serialization::pimpl::Data(*rhs.response));
                }

            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalQueueMessageType TransactionalQueueSizeCodec::REQUEST_TYPE = HZ_TRANSACTIONALQUEUE_SIZE;
                const bool TransactionalQueueSizeCodec::RETRYABLE = false;
                const char* TransactionalQueueSizeCodec::OPERATION_NAME = "TransactionalQueue.Size";
                const ResponseMessageConst TransactionalQueueSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> TransactionalQueueSizeCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalQueueSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalQueueSizeCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalQueueSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                TransactionalQueueSizeCodec::ResponseParameters
                TransactionalQueueSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalQueueSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalSetMessageType TransactionalSetAddCodec::REQUEST_TYPE = HZ_TRANSACTIONALSET_ADD;
                const bool TransactionalSetAddCodec::RETRYABLE = false;
                const char* TransactionalSetAddCodec::OPERATION_NAME = "TransactionalSet.Add";
                const ResponseMessageConst TransactionalSetAddCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalSetAddCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, item);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalSetAddCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(item);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalSetAddCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(item);
                    return dataSize;
                }

                TransactionalSetAddCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalSetAddCodec::ResponseParameters
                TransactionalSetAddCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalSetAddCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalSetMessageType TransactionalSetRemoveCodec::REQUEST_TYPE = HZ_TRANSACTIONALSET_REMOVE;
                const bool TransactionalSetRemoveCodec::RETRYABLE = false;
                const char* TransactionalSetRemoveCodec::OPERATION_NAME = "TransactionalSet.Remove";
                const ResponseMessageConst TransactionalSetRemoveCodec::RESPONSE_TYPE = (ResponseMessageConst) 101;

                std::unique_ptr<ClientMessage> TransactionalSetRemoveCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId, item);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalSetRemoveCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->set(item);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalSetRemoveCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId,
                        const serialization::pimpl::Data &item) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    dataSize += ClientMessage::calculateDataSize(item);
                    return dataSize;
                }

                TransactionalSetRemoveCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<bool>();

                }

                TransactionalSetRemoveCodec::ResponseParameters
                TransactionalSetRemoveCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalSetRemoveCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace protocol {
            namespace codec {
                const TransactionalSetMessageType TransactionalSetSizeCodec::REQUEST_TYPE = HZ_TRANSACTIONALSET_SIZE;
                const bool TransactionalSetSizeCodec::RETRYABLE = false;
                const char* TransactionalSetSizeCodec::OPERATION_NAME = "TransactionalSet.Size";
                const ResponseMessageConst TransactionalSetSizeCodec::RESPONSE_TYPE = (ResponseMessageConst) 102;

                std::unique_ptr<ClientMessage> TransactionalSetSizeCodec::encodeRequest(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t requiredDataSize = calculateDataSize(name, txnId, threadId);
                    std::unique_ptr<ClientMessage> clientMessage = ClientMessage::createForEncode(requiredDataSize);
                    clientMessage->setMessageType((uint16_t) TransactionalSetSizeCodec::REQUEST_TYPE);
                    clientMessage->setOperationName(OPERATION_NAME);
                    clientMessage->setRetryable(RETRYABLE);
                    clientMessage->set(name);
                    clientMessage->set(txnId);
                    clientMessage->set(threadId);
                    clientMessage->updateFrameLength();
                    return clientMessage;
                }

                int32_t TransactionalSetSizeCodec::calculateDataSize(
                        const std::string &name,
                        const std::string &txnId,
                        int64_t threadId) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;
                    dataSize += ClientMessage::calculateDataSize(name);
                    dataSize += ClientMessage::calculateDataSize(txnId);
                    dataSize += ClientMessage::calculateDataSize(threadId);
                    return dataSize;
                }

                TransactionalSetSizeCodec::ResponseParameters::ResponseParameters(ClientMessage &clientMessage) {


                    response = clientMessage.get<int32_t>();

                }

                TransactionalSetSizeCodec::ResponseParameters
                TransactionalSetSizeCodec::ResponseParameters::decode(ClientMessage clientMessage) {
                    return TransactionalSetSizeCodec::ResponseParameters(clientMessage);
                }


            }
        }
    }
}

