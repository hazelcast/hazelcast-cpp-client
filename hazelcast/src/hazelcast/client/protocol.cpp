/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include <assert.h>

#include "hazelcast/client/protocol/ClientMessage.h"
#include <hazelcast/client/protocol/codec/UUIDCodec.h>
#include <hazelcast/client/protocol/ClientProtocolErrorCodes.h>
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/protocol/codec/AddressCodec.h"
#include "hazelcast/client/protocol/codec/MemberCodec.h"
#include "hazelcast/client/protocol/codec/DataEntryViewCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/protocol/codec/StackTraceElementCodec.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/protocol/codec/StackTraceElement.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"
#include "hazelcast/client/protocol/Principal.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/client/impl/ClientMessageDecoder.h"
#include "hazelcast/client/serialization/PortableWriter.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            const std::string ClientTypes::CPP = "CPP";

            ClientMessage::ClientMessage(int32_t size) : LittleEndianBufferWrapper(size), retryable(false) {
                setFrameLength(size);
            }

            ClientMessage::~ClientMessage() {
            }

            void ClientMessage::wrapForEncode(int32_t size) {
                wrapForWrite(size, HEADER_SIZE);

                setFrameLength(size);
                setVersion(HAZELCAST_CLIENT_PROTOCOL_VERSION);
                addFlag(BEGIN_AND_END_FLAGS);
                setCorrelationId(0);
                setPartitionId(-1);
                setDataOffset(HEADER_SIZE);
            }

            std::unique_ptr<ClientMessage> ClientMessage::createForEncode(int32_t size) {
                std::unique_ptr<ClientMessage> msg(new ClientMessage(size));
                msg->wrapForEncode(size);
                return msg;
            }

            std::unique_ptr<ClientMessage> ClientMessage::createForDecode(const ClientMessage &msg) {
                // copy constructor does not do deep copy of underlying buffer but just uses a shared_ptr
                std::unique_ptr<ClientMessage> copy(new ClientMessage(msg));
                copy->wrapForRead(copy->getCapacity(), ClientMessage::HEADER_SIZE);
                return copy;
            }

            std::unique_ptr<ClientMessage> ClientMessage::create(int32_t size) {
                return std::unique_ptr<ClientMessage>(new ClientMessage(size));
            }

            //----- Setter methods begin --------------------------------------
            void ClientMessage::setFrameLength(int32_t length) {
                util::Bits::nativeToLittleEndian4(&length, &(*buffer)[FRAME_LENGTH_FIELD_OFFSET]);
            }

            void ClientMessage::setMessageType(uint16_t type) {
                util::Bits::nativeToLittleEndian2(&type, &(*buffer)[TYPE_FIELD_OFFSET]);
            }

            void ClientMessage::setVersion(uint8_t value) {
                (*buffer)[VERSION_FIELD_OFFSET] = value;
            }

            uint8_t ClientMessage::getFlags() {
                return (*buffer)[FLAGS_FIELD_OFFSET];
            }

            void ClientMessage::addFlag(uint8_t flags) {
                (*buffer)[FLAGS_FIELD_OFFSET] = getFlags() | flags;
            }

            void ClientMessage::setCorrelationId(int64_t id) {
                util::Bits::nativeToLittleEndian8(&id, &(*buffer)[CORRELATION_ID_FIELD_OFFSET]);
            }

            void ClientMessage::setPartitionId(int32_t partitionId) {
                util::Bits::nativeToLittleEndian4(&partitionId, &(*buffer)[PARTITION_ID_FIELD_OFFSET]);
            }

            void ClientMessage::setDataOffset(uint16_t offset) {
                util::Bits::nativeToLittleEndian2(&offset, &(*buffer)[DATA_OFFSET_FIELD_OFFSET]);
            }

            void ClientMessage::updateFrameLength() {
                setFrameLength(getIndex());
            }

            void ClientMessage::set(const std::string *value) {
                setNullable<std::string>(value);
            }


            void ClientMessage::set(const Address &data) {
                codec::AddressCodec::encode(data, *this);
            }

            void ClientMessage::set(const serialization::pimpl::Data &value) {
                setArray<byte>(value.toByteArray());
            }

            void ClientMessage::set(const serialization::pimpl::Data *value) {
                setNullable<serialization::pimpl::Data>(value);
            }

            //----- Setter methods end ---------------------

            int32_t ClientMessage::fillMessageFrom(util::ByteBuffer &byteBuff, int32_t offset, int32_t frameLen) {
                size_t numToRead = (size_t) (frameLen - offset);
                size_t numRead = byteBuff.readBytes(&(*buffer)[offset], numToRead);

                if (numRead == numToRead) {
                    wrapForRead(frameLen, ClientMessage::HEADER_SIZE);
                }

                return (int32_t) numRead;
            }

            //----- Getter methods begin -------------------
            int32_t ClientMessage::getFrameLength() const {
                int32_t result;

                util::Bits::littleEndianToNative4(&(*buffer)[FRAME_LENGTH_FIELD_OFFSET], &result);

                return result;
            }

            uint16_t ClientMessage::getMessageType() const {
                uint16_t type;

                util::Bits::littleEndianToNative2(&(*buffer)[TYPE_FIELD_OFFSET], &type);

                return type;
            }

            uint8_t ClientMessage::getVersion() {
                return (*buffer)[VERSION_FIELD_OFFSET];
            }

            int64_t ClientMessage::getCorrelationId() const {
                int64_t value;
                util::Bits::littleEndianToNative8(&(*buffer)[CORRELATION_ID_FIELD_OFFSET], &value);
                return value;
            }

            int32_t ClientMessage::getPartitionId() const {
                int32_t value;
                util::Bits::littleEndianToNative4(&(*buffer)[PARTITION_ID_FIELD_OFFSET], &value);
                return value;
            }

            uint16_t ClientMessage::getDataOffset() const {
                uint16_t value;
                util::Bits::littleEndianToNative2(&(*buffer)[DATA_OFFSET_FIELD_OFFSET], &value);
                return value;
            }

            bool ClientMessage::isFlagSet(uint8_t flag) const {
                return flag == ((*buffer)[FLAGS_FIELD_OFFSET] & flag);
            }

            template<>
            uint8_t ClientMessage::get() {
                return getUint8();
            }

            template<>
            bool ClientMessage::get() {
                return getBoolean();
            }

            template<>
            int32_t ClientMessage::get() {
                return getInt32();
            }

            template<>
            int64_t ClientMessage::get() {
                return getInt64();
            }

            template<>
            std::string ClientMessage::get() {
                return getStringUtf8();
            }

            template<>
            Address ClientMessage::get() {
                return codec::AddressCodec::decode(*this);
            }

            template<>
            util::UUID ClientMessage::get() {
                return codec::UUIDCodec::decode(*this);
            }

            template<>
            Member ClientMessage::get() {
                return codec::MemberCodec::decode(*this);
            }

            template<>
            map::DataEntryView ClientMessage::get() {
                return codec::DataEntryViewCodec::decode(*this);
            }

            template<>
            serialization::pimpl::Data ClientMessage::get() {
                int32_t len = getInt32();

                assert(checkReadAvailable(len));

                byte *start = ix();
                std::unique_ptr<std::vector<byte>> bytes = std::unique_ptr<std::vector<byte> >(
                        new std::vector<byte>(start,
                                              start +
                                              len));
                index += len;

                return serialization::pimpl::Data(bytes);
            }

            template<>
            codec::StackTraceElement ClientMessage::get() {
                return codec::StackTraceElementCodec::decode(*this);
            }

            template<>
            std::pair<serialization::pimpl::Data, serialization::pimpl::Data> ClientMessage::get() {
                serialization::pimpl::Data key = get<serialization::pimpl::Data>();
                serialization::pimpl::Data value = get<serialization::pimpl::Data>();

                return std::pair<serialization::pimpl::Data, serialization::pimpl::Data>(key, value);
            }

            template<>
            std::pair<Address, std::vector<int64_t>> ClientMessage::get() {
                Address address = codec::AddressCodec::decode(*this);
                std::vector<int64_t> values = getArray<int64_t>();
                return std::make_pair(address, values);
            }

            template<>
            std::pair<Address, std::vector<int32_t>> ClientMessage::get() {
                Address address = codec::AddressCodec::decode(*this);
                std::vector<int32_t> partitions = getArray<int32_t>();
                return std::make_pair(address, partitions);
            }

            template<>
            std::pair<std::string, int64_t> ClientMessage::get() {
                std::string key = get<std::string>();
                int64_t value = get<int64_t>();
                return std::make_pair(key, value);
            }
            //----- Getter methods end --------------------------

            //----- Data size calculation functions BEGIN -------
            int32_t ClientMessage::calculateDataSize(uint8_t param) {
                return UINT8_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(bool param) {
                return UINT8_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int32_t param) {
                return INT32_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(int64_t param) {
                return INT64_SIZE;
            }

            int32_t ClientMessage::calculateDataSize(const std::string &param) {
                return INT32_SIZE +  // bytes for the length field
                       (int32_t) param.length();
            }

            int32_t ClientMessage::calculateDataSize(const std::string *param) {
                return calculateDataSizeNullable<std::string>(param);
            }

            int32_t ClientMessage::calculateDataSize(const serialization::pimpl::Data &param) {
                return INT32_SIZE +  // bytes for the length field
                       (int32_t) param.totalSize();
            }

            int32_t ClientMessage::calculateDataSize(const serialization::pimpl::Data *param) {
                return calculateDataSizeNullable<serialization::pimpl::Data>(param);
            }
            //----- Data size calculation functions END ---------

            void ClientMessage::append(const ClientMessage *msg) {
                // no need to double check if correlation ids match here,
                // since we make sure that this is guaranteed at the caller that they are matching !
                int32_t dataSize = msg->getDataSize();
                int32_t existingFrameLen = getFrameLength();
                int32_t newFrameLen = existingFrameLen + dataSize;
                ensureBufferSize(newFrameLen);
                memcpy(&(*buffer)[existingFrameLen], &(*msg->buffer)[0], (size_t) dataSize);
                setFrameLength(newFrameLen);
            }

            int32_t ClientMessage::getDataSize() const {
                return this->getFrameLength() - getDataOffset();
            }

            void ClientMessage::ensureBufferSize(int32_t requiredCapacity) {
                int32_t currentCapacity = getCapacity();
                if (requiredCapacity > currentCapacity) {
                    // allocate new memory
                    int32_t newSize = findSuitableCapacity(requiredCapacity, currentCapacity);

                    buffer->resize(newSize, 0);
                    wrapForWrite(newSize, getIndex());
                } else {
                    // Should never be here
                    assert(0);
                }
            }

            int32_t ClientMessage::findSuitableCapacity(int32_t requiredCapacity, int32_t existingCapacity) const {
                int32_t size = existingCapacity;
                do {
                    size <<= 1;
                } while (size < requiredCapacity);

                return size;
            }

            bool ClientMessage::isRetryable() const {
                return retryable;
            }

            void ClientMessage::setRetryable(bool shouldRetry) {
                retryable = shouldRetry;
            }

            bool ClientMessage::isComplete() const {
                return (index >= HEADER_SIZE) && (index == getFrameLength());
            }

            std::ostream &operator<<(std::ostream &os, const ClientMessage &message) {
                os << "ClientMessage{length=" << message.getIndex()
                   << ", correlationId=" << message.getCorrelationId()
                   << ", messageType=0x" << std::hex << message.getMessageType() << std::dec
                   << ", partitionId=" << message.getPartitionId()
                   << ", isComplete=" << message.isComplete()
                   << ", isRetryable=" << message.isRetryable()
                   << ", isEvent=" << message.isFlagSet(message.LISTENER_EVENT_FLAG)
                   << "}";

                return os;
            }

            int32_t ClientMessage::calculateDataSize(const Address &param) {
                return codec::AddressCodec::calculateDataSize(param);
            }

            ExceptionFactory::~ExceptionFactory() {
            }

            ClientExceptionFactory::ClientExceptionFactory() {
                registerException(ARRAY_INDEX_OUT_OF_BOUNDS,
                                  new ExceptionFactoryImpl<exception::ArrayIndexOutOfBoundsException>());
                registerException(ARRAY_STORE, new ExceptionFactoryImpl<exception::ArrayStoreException>());
                registerException(AUTHENTICATIONERROR, new ExceptionFactoryImpl<exception::AuthenticationException>());
                registerException(CACHE_NOT_EXISTS, new ExceptionFactoryImpl<exception::CacheNotExistsException>());
                registerException(CALLER_NOT_MEMBER, new ExceptionFactoryImpl<exception::CallerNotMemberException>());
                registerException(CANCELLATION, new ExceptionFactoryImpl<exception::CancellationException>());
                registerException(CLASS_CAST, new ExceptionFactoryImpl<exception::ClassCastException>());
                registerException(CLASS_NOT_FOUND, new ExceptionFactoryImpl<exception::ClassNotFoundException>());
                registerException(CONCURRENT_MODIFICATION,
                                  new ExceptionFactoryImpl<exception::ConcurrentModificationException>());
                registerException(CONFIG_MISMATCH, new ExceptionFactoryImpl<exception::ConfigMismatchException>());
                registerException(CONFIGURATION, new ExceptionFactoryImpl<exception::ConfigurationException>());
                registerException(DISTRIBUTED_OBJECT_DESTROYED,
                                  new ExceptionFactoryImpl<exception::DistributedObjectDestroyedException>());
                registerException(DUPLICATE_INSTANCE_NAME,
                                  new ExceptionFactoryImpl<exception::DuplicateInstanceNameException>());
                registerException(ENDOFFILE, new ExceptionFactoryImpl<exception::EOFException>());
                registerException(EXECUTION, new ExceptionFactoryImpl<exception::ExecutionException>());
                registerException(HAZELCAST, new ExceptionFactoryImpl<exception::HazelcastException>());
                registerException(HAZELCAST_INSTANCE_NOT_ACTIVE,
                                  new ExceptionFactoryImpl<exception::HazelcastInstanceNotActiveException>());
                registerException(HAZELCAST_OVERLOAD,
                                  new ExceptionFactoryImpl<exception::HazelcastOverloadException>());
                registerException(HAZELCAST_SERIALIZATION,
                                  new ExceptionFactoryImpl<exception::HazelcastSerializationException>());
                registerException(IO, new ExceptionFactoryImpl<exception::IOException>());
                registerException(ILLEGAL_ARGUMENT, new ExceptionFactoryImpl<exception::IllegalArgumentException>());
                registerException(ILLEGAL_ACCESS_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::IllegalAccessException>());
                registerException(ILLEGAL_ACCESS_ERROR, new ExceptionFactoryImpl<exception::IllegalAccessError>());
                registerException(ILLEGAL_MONITOR_STATE,
                                  new ExceptionFactoryImpl<exception::IllegalMonitorStateException>());
                registerException(ILLEGAL_STATE, new ExceptionFactoryImpl<exception::IllegalStateException>());
                registerException(ILLEGAL_THREAD_STATE,
                                  new ExceptionFactoryImpl<exception::IllegalThreadStateException>());
                registerException(INDEX_OUT_OF_BOUNDS,
                                  new ExceptionFactoryImpl<exception::IndexOutOfBoundsException>());
                registerException(INTERRUPTED, new ExceptionFactoryImpl<exception::InterruptedException>());
                registerException(INVALID_ADDRESS, new ExceptionFactoryImpl<exception::InvalidAddressException>());
                registerException(INVALID_CONFIGURATION,
                                  new ExceptionFactoryImpl<exception::InvalidConfigurationException>());
                registerException(MEMBER_LEFT, new ExceptionFactoryImpl<exception::MemberLeftException>());
                registerException(NEGATIVE_ARRAY_SIZE,
                                  new ExceptionFactoryImpl<exception::NegativeArraySizeException>());
                registerException(NO_SUCH_ELEMENT, new ExceptionFactoryImpl<exception::NoSuchElementException>());
                registerException(NOT_SERIALIZABLE, new ExceptionFactoryImpl<exception::NotSerializableException>());
                registerException(NULL_POINTER, new ExceptionFactoryImpl<exception::NullPointerException>());
                registerException(OPERATION_TIMEOUT, new ExceptionFactoryImpl<exception::OperationTimeoutException>());
                registerException(PARTITION_MIGRATING,
                                  new ExceptionFactoryImpl<exception::PartitionMigratingException>());
                registerException(QUERY, new ExceptionFactoryImpl<exception::QueryException>());
                registerException(QUERY_RESULT_SIZE_EXCEEDED,
                                  new ExceptionFactoryImpl<exception::QueryResultSizeExceededException>());
                registerException(QUORUM, new ExceptionFactoryImpl<exception::QuorumException>());
                registerException(REACHED_MAX_SIZE, new ExceptionFactoryImpl<exception::ReachedMaxSizeException>());
                registerException(REJECTED_EXECUTION,
                                  new ExceptionFactoryImpl<exception::RejectedExecutionException>());
                registerException(REMOTE_MAP_REDUCE, new ExceptionFactoryImpl<exception::RemoteMapReduceException>());
                registerException(RESPONSE_ALREADY_SENT,
                                  new ExceptionFactoryImpl<exception::ResponseAlreadySentException>());
                registerException(RETRYABLE_HAZELCAST,
                                  new ExceptionFactoryImpl<exception::RetryableHazelcastException>());
                registerException(RETRYABLE_IO, new ExceptionFactoryImpl<exception::RetryableIOException>());
                registerException(RUNTIME, new ExceptionFactoryImpl<exception::RuntimeException>());
                registerException(SECURITY, new ExceptionFactoryImpl<exception::SecurityException>());
                registerException(SOCKET, new ExceptionFactoryImpl<exception::SocketException>());
                registerException(STALE_SEQUENCE, new ExceptionFactoryImpl<exception::StaleSequenceException>());
                registerException(TARGET_DISCONNECTED,
                                  new ExceptionFactoryImpl<exception::TargetDisconnectedException>());
                registerException(TARGET_NOT_MEMBER, new ExceptionFactoryImpl<exception::TargetNotMemberException>());
                registerException(TIMEOUT, new ExceptionFactoryImpl<exception::TimeoutException>());
                registerException(TOPIC_OVERLOAD, new ExceptionFactoryImpl<exception::TopicOverloadException>());
                registerException(TOPOLOGY_CHANGED, new ExceptionFactoryImpl<exception::TopologyChangedException>());
                registerException(TRANSACTION, new ExceptionFactoryImpl<exception::TransactionException>());
                registerException(TRANSACTION_NOT_ACTIVE,
                                  new ExceptionFactoryImpl<exception::TransactionNotActiveException>());
                registerException(TRANSACTION_TIMED_OUT,
                                  new ExceptionFactoryImpl<exception::TransactionTimedOutException>());
                registerException(URI_SYNTAX, new ExceptionFactoryImpl<exception::URISyntaxException>());
                registerException(UTF_DATA_FORMAT, new ExceptionFactoryImpl<exception::UTFDataFormatException>());
                registerException(UNSUPPORTED_OPERATION,
                                  new ExceptionFactoryImpl<exception::UnsupportedOperationException>());
                registerException(WRONG_TARGET, new ExceptionFactoryImpl<exception::WrongTargetException>());
                registerException(XA, new ExceptionFactoryImpl<exception::XAException>());
                registerException(ACCESS_CONTROL, new ExceptionFactoryImpl<exception::AccessControlException>());
                registerException(LOGIN, new ExceptionFactoryImpl<exception::LoginException>());
                registerException(UNSUPPORTED_CALLBACK,
                                  new ExceptionFactoryImpl<exception::UnsupportedCallbackException>());
                registerException(NO_DATA_MEMBER,
                                  new ExceptionFactoryImpl<exception::NoDataMemberInClusterException>());
                registerException(REPLICATED_MAP_CANT_BE_CREATED,
                                  new ExceptionFactoryImpl<exception::ReplicatedMapCantBeCreatedOnLiteMemberException>());
                registerException(MAX_MESSAGE_SIZE_EXCEEDED,
                                  new ExceptionFactoryImpl<exception::MaxMessageSizeExceeded>());
                registerException(WAN_REPLICATION_QUEUE_FULL,
                                  new ExceptionFactoryImpl<exception::WANReplicationQueueFullException>());
                registerException(ASSERTION_ERROR, new ExceptionFactoryImpl<exception::AssertionError>());
                registerException(OUT_OF_MEMORY_ERROR, new ExceptionFactoryImpl<exception::OutOfMemoryError>());
                registerException(STACK_OVERFLOW_ERROR, new ExceptionFactoryImpl<exception::StackOverflowError>());
                registerException(NATIVE_OUT_OF_MEMORY_ERROR,
                                  new ExceptionFactoryImpl<exception::NativeOutOfMemoryError>());
                registerException(SERVICE_NOT_FOUND, new ExceptionFactoryImpl<exception::ServiceNotFoundException>());
                registerException(CONSISTENCY_LOST, new ExceptionFactoryImpl<exception::ConsistencyLostException>());
            }

            ClientExceptionFactory::~ClientExceptionFactory() {
                // release memory for the factories
                for (std::map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it =
                        errorCodeToFactory.begin(); errorCodeToFactory.end() != it; ++it) {
                    delete (it->second);
                }
            }

            std::unique_ptr<exception::IException> ClientExceptionFactory::createException(const std::string &source,
                                                                                           protocol::ClientMessage &clientMessage) const {
                codec::ErrorCodec error = codec::ErrorCodec::decode(clientMessage);
                std::map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it = errorCodeToFactory.find(
                        error.errorCode);
                if (errorCodeToFactory.end() == it) {
                    return std::unique_ptr<exception::IException>(
                            new exception::UndefinedErrorCodeException(source, "",
                                                                       error.errorCode,
                                                                       clientMessage.getCorrelationId(),
                                                                       error.toString()));
                }

                return it->second->createException(source, error.message, error.toString(), error.causeErrorCode);
            }

            void ClientExceptionFactory::registerException(int32_t errorCode, ExceptionFactory *factory) {
                std::map<int, hazelcast::client::protocol::ExceptionFactory *>::iterator it = errorCodeToFactory.find(
                        errorCode);
                if (errorCodeToFactory.end() != it) {
                    char msg[100];
                    util::hz_snprintf(msg, 100, "Error code %d was already registered!!!", errorCode);
                    throw exception::IllegalStateException("ClientExceptionFactory::registerException", msg);
                }

                errorCodeToFactory[errorCode] = factory;
            }

            Principal::Principal(std::unique_ptr<std::string> &id, std::unique_ptr<std::string> &owner) : uuid(
                    std::move(id)),
                                                                                                          ownerUuid(
                                                                                                                  std::move(
                                                                                                                          owner)) {
            }

            const std::string *Principal::getUuid() const {
                return uuid.get();
            }

            const std::string *Principal::getOwnerUuid() const {
                return ownerUuid.get();
            }

            bool Principal::operator==(const Principal &rhs) const {
                if (ownerUuid.get() != NULL ? (rhs.ownerUuid.get() == NULL || *ownerUuid != *rhs.ownerUuid) :
                    ownerUuid.get() != NULL) {
                    return false;
                }

                if (uuid.get() != NULL ? (rhs.uuid.get() == NULL || *uuid != *rhs.uuid) : rhs.uuid.get() != NULL) {
                    return false;
                }

                return true;
            }

            std::ostream &operator<<(std::ostream &os, const Principal &principal) {
                os << "uuid: " << (principal.uuid.get() ? *principal.uuid : "null") << " ownerUuid: "
                   << (principal.ownerUuid ? *principal.ownerUuid : "null");
                return os;
            }

            ClientMessageBuilder::ClientMessageBuilder(connection::Connection &connection)
                    : connection(connection) {
            }

            ClientMessageBuilder::~ClientMessageBuilder() {
            }

            bool ClientMessageBuilder::onData(util::ByteBuffer &buffer) {
                bool isCompleted = false;

                if (NULL == message.get()) {
                    if (buffer.remaining() >= ClientMessage::HEADER_SIZE) {
                        util::Bits::littleEndianToNative4(
                                ((byte *) buffer.ix()) + ClientMessage::FRAME_LENGTH_FIELD_OFFSET, &frameLen);

                        message = ClientMessage::create(frameLen);
                        offset = 0;
                    }
                }

                if (NULL != message.get()) {
                    offset += message->fillMessageFrom(buffer, offset, frameLen);

                    if (offset == frameLen) {
                        if (message->isFlagSet(ClientMessage::BEGIN_AND_END_FLAGS)) {
                            //MESSAGE IS COMPLETE HERE
                            connection.handleClientMessage(std::move(message));
                            isCompleted = true;
                        } else {
                            if (message->isFlagSet(ClientMessage::BEGIN_FLAG)) {
                                // put the message into the partial messages list
                                addToPartialMessages(message);
                            } else if (message->isFlagSet(ClientMessage::END_FLAG)) {
                                // This is the intermediate frame. Append at the previous message buffer
                                appendExistingPartialMessage(message);
                                isCompleted = true;
                            }
                        }
                    }
                }

                return isCompleted;
            }

            void ClientMessageBuilder::addToPartialMessages(std::unique_ptr<ClientMessage> &message) {
                int64_t id = message->getCorrelationId();
                partialMessages[id] = std::move(message);
            }

            bool ClientMessageBuilder::appendExistingPartialMessage(std::unique_ptr<ClientMessage> &message) {
                bool result = false;

                MessageMap::iterator foundItemIter = partialMessages.find(message->getCorrelationId());
                if (partialMessages.end() != foundItemIter) {
                    foundItemIter->second->append(message.get());
                    if (message->isFlagSet(ClientMessage::END_FLAG)) {
                        // remove from message from map
                        std::shared_ptr<ClientMessage> foundMessage(foundItemIter->second);

                        partialMessages.erase(foundItemIter, foundItemIter);

                        connection.handleClientMessage(foundMessage);

                        result = true;
                    }
                } else {
                    // Should never be here
                    assert(0);
                }

                return result;
            }

            UsernamePasswordCredentials::UsernamePasswordCredentials(const std::string &principal,
                                                                     const std::string &password)
                    : principal(principal), password(password) {
            }

            const std::string &UsernamePasswordCredentials::getPrincipal() const {
                return principal;
            }

            const std::string &UsernamePasswordCredentials::getPassword() const {
                return password;
            }

            namespace codec {
                const std::string StackTraceElement::EMPTY_STRING("");

                StackTraceElement::StackTraceElement() : fileName((std::string *) NULL) {
                }

                StackTraceElement::StackTraceElement(const std::string &className, const std::string &method,
                                                     std::unique_ptr<std::string> &file, int line) : declaringClass(
                        className),
                                                                                                     methodName(
                                                                                                             method),
                                                                                                     fileName(
                                                                                                             std::move(
                                                                                                                     file)),
                                                                                                     lineNumber(
                                                                                                             line) {}


                StackTraceElement::StackTraceElement(const StackTraceElement &rhs) {
                    declaringClass = rhs.declaringClass;
                    methodName = rhs.methodName;
                    if (NULL == rhs.fileName.get()) {
                        fileName = std::unique_ptr<std::string>();
                    } else {
                        fileName = std::unique_ptr<std::string>(new std::string(*rhs.fileName));
                    }
                    lineNumber = rhs.lineNumber;
                }

                StackTraceElement &StackTraceElement::operator=(const StackTraceElement &rhs) {
                    declaringClass = rhs.declaringClass;
                    methodName = rhs.methodName;
                    if (NULL == rhs.fileName.get()) {
                        fileName = std::unique_ptr<std::string>();
                    } else {
                        fileName = std::unique_ptr<std::string>(new std::string(*rhs.fileName));
                    }
                    lineNumber = rhs.lineNumber;
                    return *this;
                }

                const std::string &StackTraceElement::getDeclaringClass() const {
                    return declaringClass;
                }

                const std::string &StackTraceElement::getMethodName() const {
                    return methodName;
                }

                const std::string &StackTraceElement::getFileName() const {
                    if (NULL == fileName.get()) {
                        return EMPTY_STRING;
                    }

                    return *fileName;
                }

                int StackTraceElement::getLineNumber() const {
                    return lineNumber;
                }

                std::ostream &operator<<(std::ostream &out, const StackTraceElement &trace) {
                    return out << trace.getFileName() << " line " << trace.getLineNumber() << " :" <<
                               trace.getDeclaringClass() << "." << trace.getMethodName();
                }

                Address AddressCodec::decode(ClientMessage &clientMessage) {
                    std::string host = clientMessage.getStringUtf8();
                    int32_t port = clientMessage.getInt32();
                    return Address(host, port);
                }

                void AddressCodec::encode(const Address &address, ClientMessage &clientMessage) {
                    clientMessage.set(address.getHost());
                    clientMessage.set((int32_t) address.getPort());
                }

                int AddressCodec::calculateDataSize(const Address &address) {
                    return ClientMessage::calculateDataSize(address.getHost()) + ClientMessage::INT32_SIZE;
                }

                ErrorCodec ErrorCodec::decode(ClientMessage &clientMessage) {
                    return ErrorCodec(clientMessage);
                }

                ErrorCodec::ErrorCodec(ClientMessage &clientMessage) {
                    assert(ErrorCodec::TYPE == clientMessage.getMessageType());

                    errorCode = clientMessage.getInt32();
                    className = clientMessage.getStringUtf8();
                    message = clientMessage.getNullable<std::string>();
                    stackTrace = clientMessage.getArray<StackTraceElement>();
                    causeErrorCode = clientMessage.getInt32();
                    causeClassName = clientMessage.getNullable<std::string>();
                }

                std::string ErrorCodec::toString() const {
                    std::ostringstream out;
                    out << "Error code:" << errorCode << ", Class name that generated the error:" << className <<
                        ", ";
                    if (NULL != message.get()) {
                        out << *message;
                    }
                    out << std::endl;
                    for (std::vector<StackTraceElement>::const_iterator it = stackTrace.begin();
                         it != stackTrace.end(); ++it) {
                        out << "\t" << (*it) << std::endl;
                    }

                    out << std::endl << "Cause error code:" << causeErrorCode << std::endl;
                    if (NULL != causeClassName.get()) {
                        out << "Caused by:" << *causeClassName;
                    }

                    return out.str();
                }

                ErrorCodec::ErrorCodec(const ErrorCodec &rhs) {
                    errorCode = rhs.errorCode;
                    className = rhs.className;
                    if (NULL != rhs.message.get()) {
                        message = std::unique_ptr<std::string>(new std::string(*rhs.message));
                    }
                    stackTrace = rhs.stackTrace;
                    causeErrorCode = rhs.causeErrorCode;
                    if (NULL != rhs.causeClassName.get()) {
                        causeClassName = std::unique_ptr<std::string>(new std::string(*rhs.causeClassName));
                    }
                }

                Member MemberCodec::decode(ClientMessage &clientMessage) {
                    Address address = AddressCodec::decode(clientMessage);
                    std::string uuid = clientMessage.get<std::string>();
                    bool liteMember = clientMessage.get<bool>();
                    int32_t attributeSize = clientMessage.get<int32_t>();
                    std::map<std::string, std::string> attributes;
                    for (int i = 0; i < attributeSize; i++) {
                        std::string key = clientMessage.get<std::string>();
                        std::string value = clientMessage.get<std::string>();
                        attributes[key] = value;
                    }

                    return Member(address, uuid, liteMember, attributes);
                }

                util::UUID UUIDCodec::decode(ClientMessage &clientMessage) {
                    return util::UUID(clientMessage.get<int64_t>(), clientMessage.get<int64_t>());
                }

                void UUIDCodec::encode(const util::UUID &uuid, ClientMessage &clientMessage) {
                    clientMessage.set(uuid.getMostSignificantBits());
                    clientMessage.set(uuid.getLeastSignificantBits());
                }

                int UUIDCodec::calculateDataSize(const util::UUID &uuid) {
                    return UUID_DATA_SIZE;
                }

                StackTraceElement StackTraceElementCodec::decode(ClientMessage &clientMessage) {
                    std::string className = clientMessage.getStringUtf8();
                    std::string methodName = clientMessage.getStringUtf8();
                    std::unique_ptr<std::string> fileName = clientMessage.getNullable<std::string>();
                    int32_t lineNumber = clientMessage.getInt32();

                    return StackTraceElement(className, methodName, fileName, lineNumber);
                }

                map::DataEntryView DataEntryViewCodec::decode(ClientMessage &clientMessage) {
                    serialization::pimpl::Data key = clientMessage.get<serialization::pimpl::Data>(); // key
                    serialization::pimpl::Data value = clientMessage.get<serialization::pimpl::Data>(); // value
                    int64_t cost = clientMessage.get<int64_t>(); // cost
                    int64_t creationTime = clientMessage.get<int64_t>(); // creationTime
                    int64_t expirationTime = clientMessage.get<int64_t>(); // expirationTime
                    int64_t hits = clientMessage.get<int64_t>(); // hits
                    int64_t lastAccessTime = clientMessage.get<int64_t>(); // lastAccessTime
                    int64_t lastStoredTime = clientMessage.get<int64_t>(); // lastStoredTime
                    int64_t lastUpdateTime = clientMessage.get<int64_t>(); // lastUpdateTime
                    int64_t version = clientMessage.get<int64_t>(); // version
                    int64_t evictionCriteria = clientMessage.get<int64_t>(); // evictionCriteriaNumber
                    int64_t ttl = clientMessage.get<int64_t>();  // ttl
                    return map::DataEntryView(key, value, cost, creationTime, expirationTime, hits, lastAccessTime,
                                              lastStoredTime, lastUpdateTime, version, evictionCriteria, ttl);
                }

                void DataEntryViewCodec::encode(const map::DataEntryView &view, ClientMessage &clientMessage) {
                    clientMessage.set(view.getKey());
                    clientMessage.set(view.getValue());
                    clientMessage.set((int64_t) view.getCost());
                    clientMessage.set((int64_t) view.getCreationTime());
                    clientMessage.set((int64_t) view.getExpirationTime());
                    clientMessage.set((int64_t) view.getHits());
                    clientMessage.set((int64_t) view.getLastAccessTime());
                    clientMessage.set((int64_t) view.getLastStoredTime());
                    clientMessage.set((int64_t) view.getLastUpdateTime());
                    clientMessage.set((int64_t) view.getVersion());
                    clientMessage.set((int64_t) view.getEvictionCriteriaNumber());
                    clientMessage.set((int64_t) view.getTtl());
                }

                int DataEntryViewCodec::calculateDataSize(const map::DataEntryView &view) {
                    int32_t dataSize = ClientMessage::HEADER_SIZE;;
                    return dataSize
                           + ClientMessage::calculateDataSize(view.getKey())
                           + ClientMessage::calculateDataSize(view.getValue())
                           + ClientMessage::INT64_SIZE * 10;
                }

            }
        }

        namespace impl {
            std::shared_ptr<void>
            VoidMessageDecoder::decodeClientMessage(const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                                                    serialization::pimpl::SerializationService &serializationService) {
                return std::shared_ptr<void>();
            }

            const std::shared_ptr<ClientMessageDecoder<void>> &VoidMessageDecoder::instance() {
                static std::shared_ptr<ClientMessageDecoder<void>> singleton(new VoidMessageDecoder);
                return singleton;
            }
        }
    }
}
