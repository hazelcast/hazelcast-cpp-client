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

#include <assert.h>

#include <boost/uuid/uuid_io.hpp>

#include "hazelcast/client/protocol/ClientMessage.h"
#include <hazelcast/client/protocol/ClientProtocolErrorCodes.h>
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            const std::string ClientTypes::CPP = "CPP";

            constexpr size_t ClientMessage::EXPECTED_DATA_BLOCK_SIZE;

            const ClientMessage::frame_header_t ClientMessage::NULL_FRAME{ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS, ClientMessage::IS_NULL_FLAG};
            const ClientMessage::frame_header_t ClientMessage::BEGIN_FRAME{ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS, ClientMessage::BEGIN_DATA_STRUCTURE_FLAG};
            const ClientMessage::frame_header_t ClientMessage::END_FRAME{ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS, ClientMessage::END_DATA_STRUCTURE_FLAG};

            ClientMessage::ClientMessage() : retryable(false) {}

            ClientMessage::ClientMessage(size_t initial_frame_size, bool is_fingle_frame) : retryable(false) {
                auto *initial_frame = reinterpret_cast<frame_header_t *>(wr_ptr(REQUEST_HEADER_LEN));
                initial_frame->frame_len = initial_frame_size;
                initial_frame->flags = is_fingle_frame ? static_cast<int16_t>(ClientMessage::UNFRAGMENTED_MESSAGE) |
                        static_cast<int16_t>(ClientMessage::IS_FINAL_FLAG) : ClientMessage::UNFRAGMENTED_MESSAGE;
            }

            void ClientMessage::wrap_for_read() {
                buffer_index = 0;
                offset = 0;
            }

            //----- Setter methods begin --------------------------------------
            void ClientMessage::setMessageType(int32_t type) {
                boost::endian::store_little_s64(&data_buffer[0][TYPE_FIELD_OFFSET], type);
            }

            void ClientMessage::setCorrelationId(int64_t id) {
                boost::endian::store_little_s64(&data_buffer[0][CORRELATION_ID_FIELD_OFFSET], id);
            }

            void ClientMessage::setPartitionId(int32_t partitionId) {
                boost::endian::store_little_s32(&data_buffer[0][PARTITION_ID_FIELD_OFFSET], partitionId);
            }

            template<>
            void ClientMessage::set(const std::vector<std::pair<boost::uuids::uuid, int64_t>> &values, bool is_final) {
                auto *f = reinterpret_cast<frame_header_t *>(wr_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                f->frame_len = values.size() * (UUID_SIZE + INT64_SIZE) + SIZE_OF_FRAME_LENGTH_AND_FLAGS;
                f->flags = is_final ? IS_FINAL_FLAG : DEFAULT_FLAGS;
                for(auto &p : values) {
                    set(p.first);
                    set(p.second);
                }
            }

            template<>
            void ClientMessage::set(const std::vector<boost::uuids::uuid> &values, bool is_final) {
                auto *h = reinterpret_cast<frame_header_t *>(wr_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                h->frame_len = SIZE_OF_FRAME_LENGTH_AND_FLAGS + values.size() * UUID_SIZE;
                h->flags = is_final ? IS_FINAL_FLAG : DEFAULT_FLAGS;
                for (auto &v : values) {
                    set(v);
                }
            }

            template<>
            void ClientMessage::set(const std::vector<int32_t> &values, bool is_final) {
                set_primitive_vector(values, is_final);
            }

            template<>
            void ClientMessage::set(const std::vector<int64_t> &values, bool is_final) {
                set_primitive_vector(values, is_final);
            }

            void ClientMessage::set(const query::anchor_data_list &list, bool is_final) {
                add_begin_frame();
                set(list.page_list);
                set(list.data_list);
                add_end_frame(is_final);
            }

            void ClientMessage::set(const codec::holder::paging_predicate_holder &p, bool is_final) {
                add_begin_frame();

                auto f = reinterpret_cast<frame_header_t *>(wr_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                f->frame_len = SIZE_OF_FRAME_LENGTH_AND_FLAGS + 2 * INT32_SIZE + INT8_SIZE;
                f->flags = DEFAULT_FLAGS;
                set(p.page_size);
                set(p.page);
                set(p.iteration_type);

                set(p.anchor_list);

                set(p.predicate_data);
                set(p.comparator_data);
                set(static_cast<serialization::pimpl::Data *>(nullptr));

                add_end_frame(is_final);
            }

            //----- Setter methods end ---------------------

            void ClientMessage::fillMessageFrom(util::ByteBuffer &byteBuff, bool &is_final, size_t &remaining_bytes_in_frame) {
                // Calculate the number of messages to read from the buffer first and then do read_bytes
                // we add the frame sizes including the final frame to find the total.
                // If there were bytes of a frame (remaining_bytes_in_frame) to read from the previous call, it is read.
                auto remaining = byteBuff.remaining();
                if (remaining_bytes_in_frame) {
                    size_t bytes_to_read = std::min(remaining_bytes_in_frame, remaining);
                    byteBuff.read_bytes(wr_ptr(bytes_to_read), bytes_to_read);
                    remaining_bytes_in_frame -= bytes_to_read;
                    if (remaining_bytes_in_frame > 0 || is_final) {
                        return;
                    }
                }

                remaining_bytes_in_frame = 0;
                // more bytes to read
                while (remaining_bytes_in_frame == 0 && !is_final && (remaining = byteBuff.remaining()) >= ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS) {
                    // start of the frame here
                    auto read_ptr = static_cast<byte *>(byteBuff.ix());
                    auto *f = reinterpret_cast<frame_header_t *>(read_ptr);
                    auto frame_len = static_cast<size_t>(static_cast<int32_t>(f->frame_len));
                    is_final = ClientMessage::is_flag_set(f->flags, ClientMessage::IS_FINAL_FLAG);
                    auto actual_bytes_to_read = std::min(frame_len, remaining);
                    byteBuff.read_bytes(wr_ptr(frame_len, actual_bytes_to_read), actual_bytes_to_read);
                    remaining_bytes_in_frame = frame_len - actual_bytes_to_read;
                }
            }

            size_t ClientMessage::size() const {
                size_t len = 0;
                for (auto &v : data_buffer) {
                    len += v.size();
                }
                return len;
            }

            int32_t ClientMessage::getMessageType() const {
                return boost::endian::load_little_s32(&data_buffer[0][TYPE_FIELD_OFFSET]);
            }

            uint16_t ClientMessage::getHeaderFlags() const {
                return boost::endian::load_little_u16(&data_buffer[0][FLAGS_FIELD_OFFSET]);
            }

            int64_t ClientMessage::getCorrelationId() const {
                return boost::endian::load_little_s64(&data_buffer[0][CORRELATION_ID_FIELD_OFFSET]);
            }

            int32_t ClientMessage::getPartitionId() const {
                return boost::endian::load_little_s32(&data_buffer[0][PARTITION_ID_FIELD_OFFSET]);
            }
/*
            void ClientMessage::append(const ClientMessage *msg) {
                // no need to double check if correlation ids match here,
                // since we make sure that this is guaranteed at the caller that they are matching !
                int32_t dataSize = msg->getDataSize();
                int32_t existingFrameLen = getFrameLength();
                int32_t newFrameLen = existingFrameLen + dataSize;
                ensureBufferSize(newFrameLen);
                memcpy(&buffer[existingFrameLen], &msg->buffer[0], (size_t) dataSize);
                setFrameLength(newFrameLen);
            }
*/

            bool ClientMessage::isRetryable() const {
                return retryable;
            }

            void ClientMessage::setRetryable(bool shouldRetry) {
                retryable = shouldRetry;
            }

            std::string ClientMessage::getOperationName() const {
                return operationName;
            }

            void ClientMessage::setOperationName(const std::string &name) {
                this->operationName = name;
            }

            std::ostream &operator<<(std::ostream &os, const ClientMessage &msg) {
                os << "ClientMessage{length=" << msg.size()
                   << ", operation=" << msg.getOperationName()
                   << ", correlationId=" << msg.getCorrelationId()
                   << ", msgType=0x" << std::hex << msg.getMessageType() << std::dec
                   << ", isRetryable=" << msg.isRetryable()
                   << ", isEvent=" << ClientMessage::is_flag_set(msg.getHeaderFlags(), ClientMessage::IS_EVENT_FLAG)
                   << "}";

                return os;
            }

            void ClientMessage::set(unsigned char *memory, boost::uuids::uuid uuid) {
                std::memcpy(wr_ptr(uuid.size()), uuid.data, uuid.size());
            }

            void ClientMessage::fast_forward_to_end_frame() {
                // We are starting from 1 because of the BEGIN_FRAME we read
                // in the beginning of the decode method
                int number_expected_frames = 1;
                while (number_expected_frames) {
                    auto *f = reinterpret_cast<frame_header_t *>(rd_ptr(sizeof(frame_header_t)));

                    int16_t flags = f->flags;
                    if (is_flag_set(flags, END_DATA_STRUCTURE_FLAG)) {
                        number_expected_frames--;
                    } else if (is_flag_set(flags, BEGIN_DATA_STRUCTURE_FLAG)) {
                        number_expected_frames++;
                    }

                    // skip current frame
                    rd_ptr(static_cast<int32_t>(f->frame_len) - sizeof(frame_header_t));
                }
            }

            ExceptionFactory::~ExceptionFactory() = default;

            ClientExceptionFactory::ClientExceptionFactory() {
                registerException(UNDEFINED,
                                  new ExceptionFactoryImpl<exception::UndefinedErrorCodeException>());
                registerException(ARRAY_INDEX_OUT_OF_BOUNDS,
                                  new ExceptionFactoryImpl<exception::ArrayIndexOutOfBoundsException>());
                registerException(ARRAY_STORE, new ExceptionFactoryImpl<exception::ArrayStoreException>());
                registerException(AUTHENTICATION, new ExceptionFactoryImpl<exception::AuthenticationException>());
                registerException(CACHE_NOT_EXISTS, new ExceptionFactoryImpl<exception::CacheNotExistsException>());
                registerException(CALLER_NOT_MEMBER, new ExceptionFactoryImpl<exception::CallerNotMemberException>());
                registerException(CANCELLATION, new ExceptionFactoryImpl<exception::CancellationException>());
                registerException(CLASS_CAST, new ExceptionFactoryImpl<exception::ClassCastException>());
                registerException(CLASS_NOT_FOUND, new ExceptionFactoryImpl<exception::ClassNotFoundException>());
                registerException(CONCURRENT_MODIFICATION,
                                  new ExceptionFactoryImpl<exception::ConcurrentModificationException>());
                registerException(CONFIG_MISMATCH, new ExceptionFactoryImpl<exception::ConfigMismatchException>());
                registerException(DISTRIBUTED_OBJECT_DESTROYED,
                                  new ExceptionFactoryImpl<exception::DistributedObjectDestroyedException>());
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
                registerException(REACHED_MAX_SIZE, new ExceptionFactoryImpl<exception::ReachedMaxSizeException>());
                registerException(REJECTED_EXECUTION,
                                  new ExceptionFactoryImpl<exception::RejectedExecutionException>());
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
                registerException(CONSISTENCY_LOST_EXCEPTION, new ExceptionFactoryImpl<exception::ConsistencyLostException>());
            }

            ClientExceptionFactory::~ClientExceptionFactory() {
                // release memory for the factories
                for (std::unordered_map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it =
                        errorCodeToFactory.begin(); errorCodeToFactory.end() != it; ++it) {
                    delete (it->second);
                }
            }

            void ClientExceptionFactory::registerException(int32_t errorCode, ExceptionFactory *factory) {
                auto it = errorCodeToFactory.find(errorCode);
                if (errorCodeToFactory.end() != it) {
                    char msg[100];
                    util::hz_snprintf(msg, 100, "Error code %d was already registered!!!", errorCode);
                    BOOST_THROW_EXCEPTION(
                            exception::IllegalStateException("ClientExceptionFactory::registerException", msg));
                }

                errorCodeToFactory[errorCode] = factory;
            }

            std::exception_ptr ClientExceptionFactory::create_exception(std::vector<codec::ErrorHolder>::const_iterator begin,
                                                          std::vector<codec::ErrorHolder>::const_iterator end) const {
                if (begin == end) {
                    return nullptr;
                }
                auto factory = errorCodeToFactory.find(begin->errorCode);
                if (errorCodeToFactory.end() == factory) {
                    factory = errorCodeToFactory.find(protocol::ClientProtocolErrorCodes::UNDEFINED);
                }
                return factory->second->create_exception(*this, begin->className, begin->message.value_or("nullptr"),
                                                  begin->toString(), create_exception(++begin, end));
            }

            std::exception_ptr ClientExceptionFactory::create_exception(const std::vector<codec::ErrorHolder> &errors) const {
                return create_exception(errors.begin(), errors.end());
            }

            ClientMessageBuilder::ClientMessageBuilder(connection::Connection &connection)
                    : connection(connection) {}

            ClientMessageBuilder::~ClientMessageBuilder() = default;

            bool ClientMessageBuilder::onData(util::ByteBuffer &buffer) {
                bool isCompleted = false;

                if (!message) {
                    message.reset(new ClientMessage());
                    is_final_frame = false;
                    remaining_frame_bytes = 0;
                }

                if (message) {
                    message->fillMessageFrom(buffer, is_final_frame, remaining_frame_bytes);
                    isCompleted = is_final_frame && remaining_frame_bytes == 0;
                    if (isCompleted) {
                        //MESSAGE IS COMPLETE HERE
                        message->wrap_for_read();
                        connection.handleClientMessage(std::move(message));
                        isCompleted = true;

                        //TODO: add fragmentation later
/*
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
*/
                    }
                }

                return isCompleted;
            }

            void ClientMessageBuilder::addToPartialMessages(std::unique_ptr<ClientMessage> &msg) {
                int64_t id = message->getCorrelationId();
                partialMessages[id] = std::move(msg);
            }

            bool ClientMessageBuilder::appendExistingPartialMessage(std::unique_ptr<ClientMessage> &msg) {
                bool result = false;

/* TODO
                MessageMap::iterator foundItemIter = partialMessages.find(msg->getCorrelationId());
                if (partialMessages.end() != foundItemIter) {
                    foundItemIter->second->append(msg.get());
                    if (msg->isFlagSet(ClientMessage::END_FLAG)) {
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
*/

                return result;
            }

            UsernamePasswordCredentials::UsernamePasswordCredentials(const std::string &principal,
                                                                     const std::string &password)
                    : name(principal), password(password) {
            }

            const std::string &UsernamePasswordCredentials::getName() const {
                return name;
            }

            const std::string &UsernamePasswordCredentials::getPassword() const {
                return password;
            }

            namespace codec {
                std::ostream &operator<<(std::ostream &out, const StackTraceElement &trace) {
                    return out << trace.fileName << " line " << trace.lineNumber << " :" << trace.declaringClass
                    << "." << trace.methodName;
                }

                std::vector<ErrorHolder> ErrorCodec::decode(ClientMessage &msg) {
                    // skip initial message frame
                    msg.skip_frame();

                    return msg.get<std::vector<ErrorHolder>>();
                }

                std::string ErrorHolder::toString() const {
                    std::ostringstream out;
                    out << "Error code:" << errorCode << ", Class name that generated the error:" << className <<
                        ", ";
                    if (message) {
                        out << *message;
                    }
                    out << std::endl;
                    for (auto s : stackTrace) {
                        out << "\t" << s << std::endl;
                    }

                    return out.str();
                }

            }
        }
    }
}
