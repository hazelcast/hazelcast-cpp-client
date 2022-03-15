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

#include <assert.h>

#include <boost/uuid/uuid_io.hpp>

#include "hazelcast/client/protocol/ClientMessage.h"
#include <hazelcast/client/protocol/ClientProtocolErrorCodes.h>
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/client/member.h"
#include "hazelcast/client/protocol/ClientExceptionFactory.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/protocol/ClientMessageBuilder.h"
#include "hazelcast/client/protocol/IMessageHandler.h"
#include "hazelcast/client/connection/Connection.h"
#include "hazelcast/client/protocol/UsernamePasswordCredentials.h"
#include "hazelcast/cp/cp.h"

namespace hazelcast {
    namespace client {
        namespace protocol {
            const std::string ClientTypes::CPP = "CPP";

            constexpr size_t ClientMessage::EXPECTED_DATA_BLOCK_SIZE;

            const ClientMessage::frame_header_t ClientMessage::NULL_FRAME{ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS, ClientMessage::IS_NULL_FLAG};
            const ClientMessage::frame_header_t ClientMessage::BEGIN_FRAME{ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS, ClientMessage::BEGIN_DATA_STRUCTURE_FLAG};
            const ClientMessage::frame_header_t ClientMessage::END_FRAME{ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS, ClientMessage::END_DATA_STRUCTURE_FLAG};

            ClientMessage::ClientMessage() : retryable_(false) {}

            ClientMessage::ClientMessage(size_t initial_frame_size, bool is_fingle_frame) : retryable_(false) {
                auto *initial_frame = reinterpret_cast<frame_header_t *>(wr_ptr(REQUEST_HEADER_LEN));
                initial_frame->frame_len = initial_frame_size;
                initial_frame->flags = is_fingle_frame ? static_cast<int16_t>(ClientMessage::UNFRAGMENTED_MESSAGE) |
                        static_cast<int16_t>(ClientMessage::IS_FINAL_FLAG) : ClientMessage::UNFRAGMENTED_MESSAGE;
            }

            void ClientMessage::wrap_for_read() {
                buffer_index_ = 0;
                offset_ = 0;
            }

            //----- Setter methods begin --------------------------------------
            void ClientMessage::set_message_type(int32_t type) {
                boost::endian::endian_store<boost::int64_t, 8, boost::endian::order::little>(
                        &data_buffer_[0][TYPE_FIELD_OFFSET], type);
            }

            void ClientMessage::set_correlation_id(int64_t id) {
                boost::endian::endian_store<boost::int64_t, 8, boost::endian::order::little>(
                        &data_buffer_[0][CORRELATION_ID_FIELD_OFFSET], id);
            }

            void ClientMessage::set_partition_id(int32_t partition_id) {
                boost::endian::endian_store<boost::int32_t, 4, boost::endian::order::little>(
                        &data_buffer_[0][PARTITION_ID_FIELD_OFFSET], partition_id);
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
            void ClientMessage::set(const std::vector<byte> &values, bool is_final) {
                set_primitive_vector(values, is_final);
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
                set(static_cast<serialization::pimpl::data *>(nullptr));

                add_end_frame(is_final);
            }

            //----- Setter methods end ---------------------

            void ClientMessage::fill_message_from(util::ByteBuffer &byte_buff, bool &is_final, size_t &remaining_bytes_in_frame) {
                // Calculate the number of messages to read from the buffer first and then do read_bytes
                // we add the frame sizes including the final frame to find the total.
                // If there were bytes of a frame (remaining_bytes_in_frame) to read from the previous call, it is read.
                auto remaining = byte_buff.remaining();
                if (remaining_bytes_in_frame) {
                    size_t bytes_to_read = (std::min)(remaining_bytes_in_frame, remaining);
                    byte_buff.read_bytes(wr_ptr(bytes_to_read), bytes_to_read);
                    remaining_bytes_in_frame -= bytes_to_read;
                    if (remaining_bytes_in_frame > 0 || is_final) {
                        return;
                    }
                }

                remaining_bytes_in_frame = 0;
                // more bytes to read
                while (remaining_bytes_in_frame == 0 && !is_final && (remaining = byte_buff.remaining()) >= ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS) {
                    // start of the frame here
                    auto read_ptr = static_cast<byte *>(byte_buff.ix());
                    auto *f = reinterpret_cast<frame_header_t *>(read_ptr);
                    auto frame_len = static_cast<size_t>(static_cast<int32_t>(f->frame_len));
                    is_final = ClientMessage::is_flag_set(f->flags, ClientMessage::IS_FINAL_FLAG);
                    auto actual_bytes_to_read = (std::min)(frame_len, remaining);
                    byte_buff.read_bytes(wr_ptr(frame_len, actual_bytes_to_read), actual_bytes_to_read);
                    remaining_bytes_in_frame = frame_len - actual_bytes_to_read;
                }
            }

            size_t ClientMessage::size() const {
                size_t len = 0;
                for (auto &v : data_buffer_) {
                    len += v.size();
                }
                return len;
            }

            int32_t ClientMessage::get_message_type() const {
                return boost::endian::endian_load<boost::int32_t, 4, boost::endian::order::little>(
                        &data_buffer_[0][TYPE_FIELD_OFFSET]);
            }

            uint16_t ClientMessage::get_header_flags() const {
                return boost::endian::endian_load<boost::uint16_t, 2, boost::endian::order::little>(
                        &data_buffer_[0][FLAGS_FIELD_OFFSET]);
            }

            void ClientMessage::set_header_flags(uint16_t new_flags) {
                return boost::endian::endian_store<boost::uint16_t, 2, boost::endian::order::little>(
                        &data_buffer_[0][FLAGS_FIELD_OFFSET], new_flags);
            }

            int64_t ClientMessage::get_correlation_id() const {
                return boost::endian::endian_load<boost::int64_t, 8, boost::endian::order::little>(
                        &data_buffer_[0][CORRELATION_ID_FIELD_OFFSET]);
            }

            int8_t ClientMessage::get_number_of_backups() const {
                return data_buffer_[0][RESPONSE_BACKUP_ACKS_FIELD_OFFSET];
            }

            int32_t ClientMessage::get_partition_id() const {
                return boost::endian::endian_load<boost::int32_t, 4, boost::endian::order::little>(
                        &data_buffer_[0][PARTITION_ID_FIELD_OFFSET]);
            }

            void ClientMessage::append(std::shared_ptr<ClientMessage> msg) {
                // no need to double check if correlation ids match here,
                // since we make sure that this is guaranteed at the caller that they are matching !
                data_buffer_.insert(data_buffer_.end(), msg->data_buffer_.begin(), msg->data_buffer_.end());
            }

            bool ClientMessage::is_retryable() const {
                return retryable_;
            }

            void ClientMessage::set_retryable(bool should_retry) {
                retryable_ = should_retry;
            }

            std::string ClientMessage::get_operation_name() const {
                return operation_name_;
            }

            void ClientMessage::set_operation_name(const std::string &name) {
                this->operation_name_ = name;
            }

            std::ostream &operator<<(std::ostream &os, const ClientMessage &msg) {
                os << "ClientMessage{length=" << msg.size()
                   << ", operation=" << msg.get_operation_name()
                   << ", isRetryable=" << msg.is_retryable();

                auto begin_fragment = msg.is_flag_set(ClientMessage::BEGIN_FRAGMENT_FLAG);
                auto unfragmented = msg.is_flag_set(ClientMessage::UNFRAGMENTED_MESSAGE);

                // print correlation id, and message type only if it is unfragmented message or
                // the first message of a fragmented message
                if (unfragmented) {
                    os << ", correlationId = " << msg.get_correlation_id()
                       << ", messageType = 0x" << std::hex << msg.get_message_type() << std::dec
                       << ", flags = 0x" << std::hex << msg.get_header_flags() << std::dec
                       << ", is backup aware = " << ClientMessage::is_flag_set(msg.get_header_flags(), ClientMessage::BACKUP_AWARE_FLAG)
                       << ", is backup event = " << ClientMessage::is_flag_set(msg.get_header_flags(), ClientMessage::BACKUP_EVENT_FLAG)
                       << ", isEvent = " << ClientMessage::is_flag_set(msg.get_header_flags(), ClientMessage::IS_EVENT_FLAG)
                       << "}";
                } else if (begin_fragment) {
                    os << ", fragmentationId = "
                       << boost::endian::endian_load<boost::int64_t, 8, boost::endian::order::little>(
                               &msg.data_buffer_[0][ClientMessage::FRAGMENTATION_ID_OFFSET])
                       << ", correlationId = " << msg.get_correlation_id()
                       << ", messageType = 0x" << std::hex << msg.get_message_type() << std::dec
                       << ", flags = 0x" << std::hex << msg.get_header_flags() << std::dec
                       << ", is backup aware = " << ClientMessage::is_flag_set(msg.get_header_flags(), ClientMessage::BACKUP_AWARE_FLAG)
                       << ", is backup event = " << ClientMessage::is_flag_set(msg.get_header_flags(), ClientMessage::BACKUP_EVENT_FLAG)
                       << ", isEvent = " << ClientMessage::is_flag_set(msg.get_header_flags(), ClientMessage::IS_EVENT_FLAG)
                       << "}";
                } else {
                    os << ", fragmentationId = "
                       << boost::endian::endian_load<boost::int64_t, 8, boost::endian::order::little>(
                               &msg.data_buffer_[0][ClientMessage::FRAGMENTATION_ID_OFFSET]);
                }
                os << ", is_fragmented = " << (unfragmented ? "no" : "yes");

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

            const ClientMessage::frame_header_t &ClientMessage::null_frame() {
                return NULL_FRAME;
            }

            const ClientMessage::frame_header_t &ClientMessage::begin_frame() {
                return BEGIN_FRAME;
            }

            const ClientMessage::frame_header_t &ClientMessage::end_frame() {
                return END_FRAME;
            }


            void ClientMessage::drop_fragmentation_frame() {
                data_buffer_[0].erase(data_buffer_[0].begin(),
                                     data_buffer_[0].begin() + FRAGMENTATION_ID_OFFSET + INT64_SIZE);
            }

            void ClientMessage::set(const cp::raft_group_id &o, bool is_final) {
                add_begin_frame();

                auto f = reinterpret_cast<frame_header_t *>(wr_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                f->frame_len = SIZE_OF_FRAME_LENGTH_AND_FLAGS + 2 * INT64_SIZE;
                f->flags = DEFAULT_FLAGS;
                set(o.seed);
                set(o.group_id);

                set(o.name);

                add_end_frame(is_final);
            }

            template<typename T>
            typename std::enable_if<std::is_same<T, cp::raft_group_id>::value, T>::type
            ClientMessage::get() {
                // skip begin frame
                rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                // skip header of the frame
                auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                auto seed = get<int64_t>();
                auto id = get<int64_t>();
                rd_ptr(static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS - 2 * INT64_SIZE);

                auto name = get<std::string>();

                fast_forward_to_end_frame();

                return {std::move(name), seed, id};
            }
            template cp::raft_group_id ClientMessage::get<cp::raft_group_id>();

            ExceptionFactory::~ExceptionFactory() = default;

            ClientExceptionFactory::ClientExceptionFactory() {
                register_exception(UNDEFINED,
                                  new ExceptionFactoryImpl<exception::undefined_error_code>());
                register_exception(ARRAY_INDEX_OUT_OF_BOUNDS,
                                  new ExceptionFactoryImpl<exception::array_index_out_of_bounds>());
                register_exception(ARRAY_STORE, new ExceptionFactoryImpl<exception::array_store>());
                register_exception(AUTHENTICATION, new ExceptionFactoryImpl<exception::authentication>());
                register_exception(CACHE_NOT_EXISTS, new ExceptionFactoryImpl<exception::cache_not_exists>());
                register_exception(CALLER_NOT_MEMBER, new ExceptionFactoryImpl<exception::caller_not_member>());
                register_exception(CANCELLATION, new ExceptionFactoryImpl<exception::cancellation>());
                register_exception(CLASS_CAST, new ExceptionFactoryImpl<exception::class_cast>());
                register_exception(CLASS_NOT_FOUND, new ExceptionFactoryImpl<exception::class_not_found>());
                register_exception(CONCURRENT_MODIFICATION,
                                  new ExceptionFactoryImpl<exception::concurrent_modification>());
                register_exception(CONFIG_MISMATCH, new ExceptionFactoryImpl<exception::config_mismatch>());
                register_exception(DISTRIBUTED_OBJECT_DESTROYED,
                                  new ExceptionFactoryImpl<exception::distributed_object_destroyed>());
                register_exception(ENDOFFILE, new ExceptionFactoryImpl<exception::eof>());
                register_exception(EXECUTION, new ExceptionFactoryImpl<exception::execution>());
                register_exception(HAZELCAST, new ExceptionFactoryImpl<exception::hazelcast_>());
                register_exception(HAZELCAST_INSTANCE_NOT_ACTIVE,
                                  new ExceptionFactoryImpl<exception::hazelcast_instance_not_active>());
                register_exception(HAZELCAST_OVERLOAD,
                                  new ExceptionFactoryImpl<exception::hazelcast_overload>());
                register_exception(HAZELCAST_SERIALIZATION,
                                  new ExceptionFactoryImpl<exception::hazelcast_serialization>());
                register_exception(IO, new ExceptionFactoryImpl<exception::io>());
                register_exception(ILLEGAL_ARGUMENT, new ExceptionFactoryImpl<exception::illegal_argument>());
                register_exception(ILLEGAL_ACCESS_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::illegal_access>());
                register_exception(ILLEGAL_ACCESS_ERROR, new ExceptionFactoryImpl<exception::illegal_access_error>());
                register_exception(ILLEGAL_MONITOR_STATE,
                                  new ExceptionFactoryImpl<exception::illegal_monitor_state>());
                register_exception(ILLEGAL_STATE, new ExceptionFactoryImpl<exception::illegal_state>());
                register_exception(ILLEGAL_THREAD_STATE,
                                  new ExceptionFactoryImpl<exception::illegal_thread_state>());
                register_exception(INDEX_OUT_OF_BOUNDS,
                                  new ExceptionFactoryImpl<exception::index_out_of_bounds>());
                register_exception(INTERRUPTED, new ExceptionFactoryImpl<exception::interrupted>());
                register_exception(INVALID_ADDRESS, new ExceptionFactoryImpl<exception::invalid_address>());
                register_exception(INVALID_CONFIGURATION,
                                  new ExceptionFactoryImpl<exception::invalid_configuration>());
                register_exception(MEMBER_LEFT, new ExceptionFactoryImpl<exception::member_left>());
                register_exception(NEGATIVE_ARRAY_SIZE,
                                  new ExceptionFactoryImpl<exception::negative_array_size>());
                register_exception(NO_SUCH_ELEMENT, new ExceptionFactoryImpl<exception::no_such_element>());
                register_exception(NOT_SERIALIZABLE, new ExceptionFactoryImpl<exception::not_serializable>());
                register_exception(NULL_POINTER, new ExceptionFactoryImpl<exception::null_pointer>());
                register_exception(OPERATION_TIMEOUT, new ExceptionFactoryImpl<exception::operation_timeout>());
                register_exception(PARTITION_MIGRATING,
                                  new ExceptionFactoryImpl<exception::partition_migrating>());
                register_exception(QUERY, new ExceptionFactoryImpl<exception::query>());
                register_exception(QUERY_RESULT_SIZE_EXCEEDED,
                                  new ExceptionFactoryImpl<exception::query_result_size_exceeded>());
                register_exception(REACHED_MAX_SIZE, new ExceptionFactoryImpl<exception::reached_max_size>());
                register_exception(REJECTED_EXECUTION,
                                  new ExceptionFactoryImpl<exception::rejected_execution>());
                register_exception(RESPONSE_ALREADY_SENT,
                                  new ExceptionFactoryImpl<exception::response_already_sent>());
                register_exception(RETRYABLE_HAZELCAST,
                                  new ExceptionFactoryImpl<exception::retryable_hazelcast>());
                register_exception(RETRYABLE_IO, new ExceptionFactoryImpl<exception::retryable_io>());
                register_exception(RUNTIME, new ExceptionFactoryImpl<exception::runtime>());
                register_exception(SECURITY, new ExceptionFactoryImpl<exception::SecurityException>());
                register_exception(SOCK_ERROR, new ExceptionFactoryImpl<exception::socket>());
                register_exception(STALE_SEQUENCE, new ExceptionFactoryImpl<exception::stale_sequence>());
                register_exception(TARGET_DISCONNECTED,
                                  new ExceptionFactoryImpl<exception::target_disconnected>());
                register_exception(TARGET_NOT_MEMBER, new ExceptionFactoryImpl<exception::target_not_member>());
                register_exception(TIMEOUT, new ExceptionFactoryImpl<exception::timeout>());
                register_exception(TOPIC_OVERLOAD, new ExceptionFactoryImpl<exception::topic_overload>());
                register_exception(TRANSACTION, new ExceptionFactoryImpl<exception::transaction>());
                register_exception(TRANSACTION_NOT_ACTIVE,
                                  new ExceptionFactoryImpl<exception::transaction_not_active>());
                register_exception(TRANSACTION_TIMED_OUT,
                                  new ExceptionFactoryImpl<exception::transaction_timed_out>());
                register_exception(URI_SYNTAX, new ExceptionFactoryImpl<exception::uri_syntax>());
                register_exception(UTF_DATA_FORMAT, new ExceptionFactoryImpl<exception::utf_data_format>());
                register_exception(UNSUPPORTED_OPERATION,
                                  new ExceptionFactoryImpl<exception::unsupported_operation>());
                register_exception(WRONG_TARGET, new ExceptionFactoryImpl<exception::wrong_target>());
                register_exception(XA, new ExceptionFactoryImpl<exception::xa>());
                register_exception(ACCESS_CONTROL, new ExceptionFactoryImpl<exception::access_control>());
                register_exception(LOGIN, new ExceptionFactoryImpl<exception::login>());
                register_exception(UNSUPPORTED_CALLBACK,
                                  new ExceptionFactoryImpl<exception::unsupported_callback>());
                register_exception(NO_DATA_MEMBER,
                                  new ExceptionFactoryImpl<exception::no_data_member_in_cluster>());
                register_exception(REPLICATED_MAP_CANT_BE_CREATED,
                                  new ExceptionFactoryImpl<exception::replicated_map_cant_be_created_on_lite_member>());
                register_exception(MAX_MESSAGE_SIZE_EXCEEDED,
                                  new ExceptionFactoryImpl<exception::max_message_size_exceeded>());
                register_exception(WAN_REPLICATION_QUEUE_FULL,
                                  new ExceptionFactoryImpl<exception::wan_replication_queue_full>());
                register_exception(ASSERTION_ERROR, new ExceptionFactoryImpl<exception::assertion_error>());
                register_exception(OUT_OF_MEMORY_ERROR, new ExceptionFactoryImpl<exception::out_of_memory_error>());
                register_exception(STACK_OVERFLOW_ERROR, new ExceptionFactoryImpl<exception::stack_overflow_error>());
                register_exception(NATIVE_OUT_OF_MEMORY_ERROR,
                                  new ExceptionFactoryImpl<exception::native_out_of_memory_error>());
                register_exception(SERVICE_NOT_FOUND, new ExceptionFactoryImpl<exception::service_not_found>());
                register_exception(STALE_TASK_ID, new ExceptionFactoryImpl<exception::stale_task_id>());
                register_exception(DUPLICATE_TASK, new ExceptionFactoryImpl<exception::duplicate_task>());
                register_exception(STALE_TASK, new ExceptionFactoryImpl<exception::stale_task>());
                register_exception(LOCAL_MEMBER_RESET, new ExceptionFactoryImpl<exception::local_member_reset>());
                register_exception(INDETERMINATE_OPERATION_STATE,
                                  new ExceptionFactoryImpl<exception::indeterminate_operation_state>());
                register_exception(FLAKE_ID_NODE_ID_OUT_OF_RANGE_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::node_id_out_of_range>());
                register_exception(TARGET_NOT_REPLICA_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::target_not_replica>());
                register_exception(MUTATION_DISALLOWED_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::mutation_disallowed>());
                register_exception(CONSISTENCY_LOST_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::consistency_lost>());
                register_exception(SESSION_EXPIRED_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::session_expired>());
                register_exception(WAIT_KEY_CANCELLED_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::wait_key_cancelled>());
                register_exception(LOCK_ACQUIRE_LIMIT_REACHED_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::lock_acquire_limit_reached>());
                register_exception(LOCK_OWNERSHIP_LOST_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::lock_ownership_lost>());
                register_exception(CP_GROUP_DESTROYED_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::cp_group_destroyed>());
                register_exception(CANNOT_REPLICATE_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::cannot_replicate>());
                register_exception(LEADER_DEMOTED_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::leader_demoted>());
                register_exception(STALE_APPEND_REQUEST_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::stale_append_request>());
                register_exception(NOT_LEADER_EXCEPTION, new ExceptionFactoryImpl<exception::not_leader>());
                register_exception(VERSION_MISMATCH_EXCEPTION,
                                  new ExceptionFactoryImpl<exception::version_mismatch>());
            }

            ClientExceptionFactory::~ClientExceptionFactory() {
                // release memory for the factories
                for (std::unordered_map<int, hazelcast::client::protocol::ExceptionFactory *>::const_iterator it =
                        error_code_to_factory_.begin(); error_code_to_factory_.end() != it; ++it) {
                    delete (it->second);
                }
            }

            void ClientExceptionFactory::register_exception(int32_t error_code, ExceptionFactory *factory) {
                auto it = error_code_to_factory_.find(error_code);
                if (error_code_to_factory_.end() != it) {
                    char msg[100];
                    util::hz_snprintf(msg, 100, "Error code %d was already registered!!!", error_code);
                    BOOST_THROW_EXCEPTION(
                            exception::illegal_state("ClientExceptionFactory::registerException", msg));
                }

                error_code_to_factory_[error_code] = factory;
            }

            std::exception_ptr ClientExceptionFactory::create_exception(std::vector<codec::ErrorHolder>::const_iterator begin,
                                                          std::vector<codec::ErrorHolder>::const_iterator end) const {
                if (begin == end) {
                    return nullptr;
                }
                auto factory = error_code_to_factory_.find(begin->error_code);
                if (error_code_to_factory_.end() == factory) {
                    factory = error_code_to_factory_.find(protocol::client_protocol_error_codes::UNDEFINED);
                }
                return factory->second->create_exception(*this, begin->class_name, begin->message.value_or("nullptr"),
                                                  begin->to_string(), create_exception(begin + 1, end));
            }

            std::exception_ptr ClientExceptionFactory::create_exception(const std::vector<codec::ErrorHolder> &errors) const {
                return create_exception(errors.begin(), errors.end());
            }

            UsernamePasswordCredentials::UsernamePasswordCredentials(const std::string &principal,
                                                                     const std::string &password)
                    : name_(principal), password_(password) {
            }

            const std::string &UsernamePasswordCredentials::get_name() const {
                return name_;
            }

            const std::string &UsernamePasswordCredentials::get_password() const {
                return password_;
            }

            namespace codec {
                std::ostream &operator<<(std::ostream &out, const StackTraceElement &trace) {
                    return out << trace.file_name << " line " << trace.line_number << " :" << trace.declaring_class
                    << "." << trace.method_name;
                }

                std::vector<ErrorHolder> ErrorCodec::decode(ClientMessage &msg) {
                    // skip initial message frame
                    msg.skip_frame();

                    return msg.get<std::vector<ErrorHolder>>();
                }

                std::string ErrorHolder::to_string() const {
                    std::ostringstream out;
                    out << "Error code:" << error_code << ", Class name that generated the error:" << class_name <<
                        ", ";
                    if (message) {
                        out << *message;
                    }
                    out << std::endl;
                    for (auto s : stack_trace) {
                        out << "\t" << s << std::endl;
                    }

                    return out.str();
                }

            }
        }
    }
}
