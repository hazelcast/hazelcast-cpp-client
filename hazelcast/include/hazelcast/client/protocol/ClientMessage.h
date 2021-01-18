/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <string>
#include <memory>
#include <vector>
#include <assert.h>
#include <unordered_map>
#include <ostream>
#include <boost/uuid/uuid.hpp>
#include <boost/endian/arithmetic.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/optional.hpp>
#include <boost/uuid/nil_generator.hpp>

#include <hazelcast/client/query/paging_predicate.h>
#include "hazelcast/client/address.h"
#include "hazelcast/client/member.h"
#include "hazelcast/client/serialization/pimpl/data.h"
#include "hazelcast/client/map/data_entry_view.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/client/config/index_config.h"
#include "hazelcast/client/protocol/codec/ErrorCodec.h"

namespace hazelcast {
    namespace util {
        class ByteBuffer;
    }

    namespace cp {
        struct raft_group_id;
    }
    namespace client {
        namespace protocol {

            class HAZELCAST_API ClientTypes {
            public:
                static const std::string CPP;
            };

            namespace codec {
                namespace holder {
                    struct HAZELCAST_API paging_predicate_holder {
                        int32_t page_size;
                        int32_t page;
                        byte iteration_type;

                        const query::anchor_data_list &anchor_list;
                        const serialization::pimpl::data *predicate_data;
                        const serialization::pimpl::data *comparator_data;

                        template<typename K, typename V>
                        static paging_predicate_holder
                        of(const query::paging_predicate <K, V> &p, serialization::pimpl::SerializationService &ss) {
                            return {static_cast<int32_t>(p.get_page_size()), static_cast<int32_t>(p.get_page()),
                                    static_cast<byte>(p.get_iteration_type()), p.anchor_data_list_,
                                    p.predicate_data_.get_ptr(), p.comparator_data_.get_ptr()
                            };
                        }
                    };
                }
            }

            template <typename>
            struct HAZELCAST_API is_trivial_entry_vector : std::false_type
            { };

            template <typename T, typename U>
            struct is_trivial_entry_vector<std::vector<std::pair<T, U>>> : std::true_type
            { };

            template <>
            struct HAZELCAST_API is_trivial_entry_vector<std::vector<std::pair<serialization::pimpl::data, boost::optional<hazelcast::client::serialization::pimpl::data>>>> : std::false_type
            { };

            template <>
            struct HAZELCAST_API is_trivial_entry_vector<std::vector<std::pair<serialization::pimpl::data, hazelcast::client::serialization::pimpl::data>>> : std::false_type
            { };

            /**
             * Client Message is the carrier framed data as defined below.
             * Any request parameter, response or event data will be carried in
             * the payload.
             *
             * client-message               = message-first-frame *var-sized-param
             * message-first-frame          = frame-length flags message-type correlation-id *fix-sized-param
             * first-frame-flags            = %b1 %b1 %b0 13unused ; begin-fragment:1 end-fragment:1 final:0 ......
             * frame-length                 = int32
             * message-type                 = int32
             * correlation-id               = int64
             *
             * var-sized-param              = string-frame / custom-type-frames / var-sized-param-list-frames / fixed-sized-param-list-frame
             * / map-fixed-to-fixed-frame / map-var-sized-to-var-sized-frames / null-frame
             *
             * map-fixed-to-fixed-frame          = frame-length flags *fixed-size-entry
             * fixed-size-entry                  = fixed-sized-param fixed-sized-param
             * map-var-sized-to-var-sized-frames = begin-frame *var-sized-entry end-frame
             * var-sized-entry                   = var-sized-param var-sized-param
             *
             * //map-fixed-sized-to-var-sized-frames // Not defined yet. Has no usage yet.
             * //map-var-sized-to-fixed-sized-frames // Not defined yet. Has no usage yet.
             *
             * list-frames                  = var-sized-param-list-frames | fixed-sized-param-list-frame
             * var-sized-param-list-frames  = begin-frame *var-sized-param  end-frame  ; all elements should be same type
             * fixed-sized-param-list-frame = frame-length flags *fixed-sized-param    ; all elements should be same type
             *
             *
             * string-frame                 = frame-length flags *OCTET ; Contains UTF-8 encoded octets
             *
             * custom-type-frames           = begin-frame *1custom-type-first-frame *var-sized-param end-frame
             * custom-type-first-frame      = frame-length flags *fix-sized-param
             *
             *
             * null-frame                   = %x00 %x00 %x00 %x05 null-flags
             * null-flags                   = %b0  %b0  %b0 %b0 %b0 %b1 10reserved  ; is-null: 1
             * ; frame-length is always 5
             * begin-frame                  = %x00 %x00 %x00 %x05 begin-flags
             * ; begin data structure: 1, end data structure: 0
             * begin-flags                  = begin-fragment end-fragment final %b1 %b0 is-null 10reserved
             * ; frame-length is always 5
             * end-frame                    = %x00 %x00 %x00 %x05 end-flags
             * ; next:0 or 1, begin list: 0, end list: 1
             * end-flags                    = begin-fragment end-fragment final %b1 %b0 is-null 10reserved
             *
             * flags          = begin-fragment end-fragment final begin-data-structure end-data-structure is-null is-event 9reserved
             * ; reserved for fragmentation
             * begin-fragment = BIT
             * ; reserved for fragmentation
             * end-fragment   = BIT
             * ; set to 1 when this frame is the last frame of the client-message
             * final          = BIT
             * ; set to 1 if this frame represents a null field.
             * is-null        = BIT
             * ; set to 1 if this is a begin-frame. begin-frame represents begin of a custom-type or a variable-field-list, 0 otherwise
             * begin          = BIT
             * ; set to 1 if this an end-frame. end-frame represents end of a custom-type or a variable-field-list, 0 otherwise
             * end            = BIT
             * ; Reserved for future usage.
             * reserved       = BIT
             * ; Irrelevant int this context
             * unused         = BIT
             * is-event       = BIT ;
             *
             * fixed-sized-param        = *OCTET
             * ;fixed-sized-param       = OCTET / boolean / int16 / int32 / int64 / UUID
             * ;boolean                 = %x00 / %x01
             * ;int16                   = 16BIT
             * ;int32                   = 32BIT
             * ;int64                   = 64BIT
             * ;UUID                    = int64 int64
             */
            class HAZELCAST_API ClientMessage {
            public:
                static constexpr size_t EXPECTED_DATA_BLOCK_SIZE = 1024;

                enum type_sizes {
                    INT8_SIZE = 1,
                    UINT8_SIZE = 1,
                    INT16_SIZE = 2,
                    UINT16_SIZE = 2,
                    INT32_SIZE = 4,
                    UINT32_SIZE = 4,
                    UINT64_SIZE = 8,
                    INT64_SIZE = 8,
                    UUID_SIZE = 17
                };

                enum flags_t {
                    DEFAULT_FLAGS = 0,
                    BEGIN_FRAGMENT_FLAG = 1 << 15,
                    END_FRAGMENT_FLAG = 1 << 14,
                    UNFRAGMENTED_MESSAGE = BEGIN_FRAGMENT_FLAG | END_FRAGMENT_FLAG,
                    IS_FINAL_FLAG = 1 << 13,
                    BEGIN_DATA_STRUCTURE_FLAG = 1 << 12,
                    END_DATA_STRUCTURE_FLAG = 1 << 11,
                    IS_NULL_FLAG = 1 << 10,
                    IS_EVENT_FLAG = 1 << 9,
                    BACKUP_AWARE_FLAG = 1 << 8,
                    BACKUP_EVENT_FLAG = 1 << 7
                };

                struct HAZELCAST_API frame_header_t {
                    boost::endian::little_int32_t frame_len;
                    boost::endian::little_int16_t flags;

                    friend bool HAZELCAST_API operator==(const frame_header_t &lhs, const frame_header_t &rhs) {
                        return lhs.frame_len == rhs.frame_len &&
                               lhs.flags == rhs.flags;
                    }
                };

                //frame length + flags
                static constexpr size_t SIZE_OF_FRAME_LENGTH_AND_FLAGS = INT32_SIZE + UINT16_SIZE;
                static constexpr size_t FLAGS_FIELD_OFFSET = INT32_SIZE;

                static constexpr size_t TYPE_FIELD_OFFSET = SIZE_OF_FRAME_LENGTH_AND_FLAGS;
                static constexpr size_t CORRELATION_ID_FIELD_OFFSET = TYPE_FIELD_OFFSET + INT32_SIZE;
                //backup acks field offset is used by response messages
                static constexpr size_t RESPONSE_BACKUP_ACKS_FIELD_OFFSET = CORRELATION_ID_FIELD_OFFSET + INT64_SIZE;
                //partition id field offset used by request and event messages
                static constexpr size_t PARTITION_ID_FIELD_OFFSET = CORRELATION_ID_FIELD_OFFSET + INT64_SIZE;
                static constexpr size_t REQUEST_HEADER_LEN = PARTITION_ID_FIELD_OFFSET + INT32_SIZE;
                static constexpr size_t EVENT_HEADER_LEN = PARTITION_ID_FIELD_OFFSET + INT32_SIZE;
                static constexpr size_t RESPONSE_HEADER_LEN = RESPONSE_BACKUP_ACKS_FIELD_OFFSET + INT8_SIZE;
                //offset valid for fragmentation frames only
                static constexpr size_t FRAGMENTATION_ID_OFFSET = SIZE_OF_FRAME_LENGTH_AND_FLAGS;

                ClientMessage();

                explicit ClientMessage(size_t initial_frame_size, bool is_fingle_frame = false);

                const std::vector<std::vector<byte>> &get_buffer() const {
                    return data_buffer_;
                }

                void wrap_for_read();

                inline byte *wr_ptr(size_t requested_bytes) {
                    return wr_ptr(requested_bytes, requested_bytes);
                }

                inline byte *wr_ptr(size_t bytes_to_reserve, size_t actual_number_of_bytes) {
                    assert(bytes_to_reserve >= actual_number_of_bytes);
                    size_t max_available_bytes = 0;
                    auto b = data_buffer_.rbegin();
                    if (b != data_buffer_.rend()) {
                        max_available_bytes = b->capacity() - b->size();
                    }
                    if (max_available_bytes < bytes_to_reserve) {
                        // add a new buffer enough size to hold the minimum requested bytes
                        data_buffer_.emplace_back();
                        b = data_buffer_.rbegin();
                        b->reserve((std::max)(EXPECTED_DATA_BLOCK_SIZE, bytes_to_reserve));
                    }

                    auto position = b->size();
                    b->insert(b->end(), actual_number_of_bytes, 0);
                    return &(*b)[position];
                }

                inline byte *rd_ptr(size_t requested_bytes) {
                    byte *result = peek(requested_bytes);
                    offset_ += requested_bytes;
                    return result;
                }

                inline void seek(size_t position) {
                    assert(buffer_index_ == 0 && position >= offset_ && position < data_buffer_[buffer_index_].size());
                    offset_ = position;
                }

                inline byte *peek(size_t requested_bytes) {
                    if(requested_bytes <= 0) {
                        return nullptr;
                    }

                    if (offset_ >= data_buffer_[buffer_index_].size()) {
                        ++buffer_index_;
                        if (buffer_index_ == data_buffer_.size()) {
                            BOOST_THROW_EXCEPTION(client::exception::hazelcast_serialization("peek",
                                                                                                       (boost::format("Not enough bytes in client message to read. Requested %1% bytes but "
                                                                                                                    "there is no more bytes left to read. %2%") %requested_bytes %*this).str()));
                        }

                        offset_ = 0;
                    }

                    if (offset_ + requested_bytes > data_buffer_[buffer_index_].size()) {
                        BOOST_THROW_EXCEPTION(client::exception::hazelcast_serialization("peek",
                                                                                                   (boost::format("Not enough bytes in client message to read. Requested %1% bytes but there "
                                                                                                                "is not enough bytes left to read. %2%") %requested_bytes %*this).str()));
                    }

                    return &data_buffer_[buffer_index_][offset_];
                }

                //---------------------- Getters -------------------------------
                template<typename T>
                typename std::enable_if<std::is_same<T, uint8_t>::value, T>::type
                inline get() {
                    return *rd_ptr(UINT8_SIZE);
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, int8_t>::value, T>::type
                inline get() {
                    return *rd_ptr(UINT8_SIZE);
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, bool>::value, T>::type
                inline get() {
                    return *rd_ptr(UINT8_SIZE);
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, uint16_t>::value, T>::type
                inline get() {
                    return boost::endian::endian_load<boost::uint16_t, 2, boost::endian::order::little>(
                            rd_ptr(UINT16_SIZE));
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, int16_t>::value, T>::type
                inline get() {
                    return boost::endian::endian_load<boost::int16_t, 2, boost::endian::order::little>(
                            rd_ptr(INT16_SIZE));
                }

                inline uint32_t get_uint32() {
                    return boost::endian::endian_load<boost::uint32_t, 4, boost::endian::order::little>(
                            rd_ptr(UINT32_SIZE));
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, int32_t>::value, T>::type
                inline get() {
                    return boost::endian::endian_load<boost::int32_t, 4, boost::endian::order::little>(
                            rd_ptr(INT32_SIZE));
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, uint64_t>::value, T>::type
                inline get() {
                    return boost::endian::endian_load<boost::uint64_t, 8, boost::endian::order::little>(
                            rd_ptr(UINT64_SIZE));
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, int64_t>::value, T>::type
                inline get() {
                    return boost::endian::endian_load<boost::int64_t, 8, boost::endian::order::little>(
                            rd_ptr(INT64_SIZE));
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, std::string>::value, T>::type
                inline get() {
                    int32_t len = get<int32_t>();
                    //skip flags
                    rd_ptr(INT16_SIZE);
                    auto str_bytes_len = len - ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS;
                    return std::string(reinterpret_cast<const char *>(rd_ptr(str_bytes_len)), str_bytes_len);
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, std::vector<typename T::value_type>>::value &&
                        !std::is_trivial<typename T::value_type>::value && !is_trivial_entry_vector<T>::value, T>::type
                get() {
                    T result;
                    // skip begin frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    while(!next_frame_is_data_structure_end_frame()) {
                        result.emplace_back(get<typename T::value_type>());
                    }

                    // skip end frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    return result;
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, std::vector<typename T::value_type>>::value &&
                        std::is_trivial<typename T::value_type>::value, T>::type
                get() {
                    T result;

                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    auto content_length = static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS;
                    size_t item_count = content_length / ClientMessage::get_sizeof<typename T::value_type>();
                    for (size_t i = 0; i < item_count; ++i) {
                        result.push_back(get<typename T::value_type>());
                    }

                    return result;
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, std::vector<typename T::value_type>>::value &&
                        std::is_same<std::pair<typename T::value_type::first_type, typename T::value_type::second_type>, typename T::value_type>::value &&
                        std::is_trivial<typename T::value_type::first_type>::value && std::is_trivial<typename T::value_type::second_type>::value, T>::type
                get() {
                    T result;

                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    auto content_length = static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS;
                    size_t item_count = content_length / (ClientMessage::get_sizeof<typename T::value_type::first_type>() + ClientMessage::get_sizeof<typename T::value_type::second_type>());
                    for (size_t i = 0; i < item_count; ++i) {
                        auto key = get<typename T::value_type::first_type>();
                        auto value = get<typename T::value_type::second_type>();
                        result.emplace_back(std::make_pair(std::move(key), std::move(value)));
                    }

                    return result;

                }

                template<typename T>
                typename std::enable_if<std::is_same<T, std::vector<typename T::value_type>>::value &&
                        std::is_same<std::pair<typename T::value_type::first_type, typename T::value_type::second_type>, typename T::value_type>::value &&
                        std::is_trivial<typename T::value_type::first_type>::value && !std::is_trivial<typename T::value_type::second_type>::value, T>::type
                get() {
                    T result;

                    auto values = get<std::vector<typename T::value_type::second_type>>();
                    auto keys = get<std::vector<typename T::value_type::first_type>>();

                    for (size_t i = 0; i < keys.size(); ++i) {
                        result.emplace_back(std::make_pair(keys[i], std::move(values[i])));
                    }

                    return result;

                }

                template<typename T>
                typename std::enable_if<std::is_same<T, std::unordered_map<typename T::key_type, typename T::mapped_type>>::value, T>::type
                inline get() {
                    // skip begin frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    T result;
                    while(!next_frame_is_data_structure_end_frame()) {
                        auto key = get<typename T::key_type>();
                        auto value = get<typename T::mapped_type>();
                        result.emplace(std::move(key), std::move(value));
                    }

                    // skip end frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    return result;
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, address>::value, T>::type
                inline get() {
                    // skip begin frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    auto port = get<int32_t>();
                    // skip bytes in initial frame
                    rd_ptr(static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS - INT32_SIZE);

                    auto host = get<std::string>();

                    fast_forward_to_end_frame();

                    return address(host, port);
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, member>::value, T>::type
                inline get() {
                    // skip begin frame
                    rd_ptr(ClientMessage::SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    // initial frame
                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    auto uuid = get<boost::uuids::uuid>();
                    auto lite_member = get<bool>();
                    // skip rest of the bytes in initial frame
                    rd_ptr(static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS - UUID_SIZE - UINT8_SIZE);

                    auto addr = get<address>();
                    auto attributes = get<std::unordered_map<std::string, std::string>>();

                    fast_forward_to_end_frame();

                    return member(std::move(addr), uuid, lite_member, std::move(attributes));
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, boost::uuids::uuid>::value, T>::type
                inline get() {
                    if (get<bool>()) {
                        // skip the next 16 bytes
                        rd_ptr(sizeof(boost::uuids::uuid));
                        return boost::uuids::nil_uuid();
                    }
                    return get_uuid();
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, serialization::pimpl::data>::value, T>::type
                inline get() {
                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    auto data_size = static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS;
                    auto mem_ptr = rd_ptr(data_size);
                    std::vector<byte> bytes(data_size);
                    std::memcpy(&bytes[0], mem_ptr, data_size);
                    return serialization::pimpl::data(std::move(bytes));
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, codec::StackTraceElement>::value, T>::type
                inline get() {
                    // skip begin frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    auto line_number = get<int32_t>();
                    // skip bytes in initial frame
                    rd_ptr(static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS - INT32_SIZE);

                    auto class_name = get<std::string>();
                    auto method_name = get<std::string>();
                    auto file_name = get_nullable<std::string>();

                    fast_forward_to_end_frame();

                    return codec::StackTraceElement{class_name, method_name, file_name, line_number};
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, query::anchor_data_list>::value, T>::type
                inline get() {
                    // skip begin frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    auto page_list = get<decltype(query::anchor_data_list::page_list)>();
                    auto data_list = get<decltype(query::anchor_data_list::data_list)>();

                    fast_forward_to_end_frame();

                    return {std::move(page_list), std::move(data_list)};
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, map::data_entry_view>::value, T>::type
                inline get() {
                    // skip begin frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));

                    auto cost = get<int64_t>();
                    auto creationTime = get<int64_t>();
                    auto expirationTime = get<int64_t>();
                    auto hits = get<int64_t>();
                    auto lastAccessTime = get<int64_t>();
                    auto lastStoredTime = get<int64_t>();
                    auto lastUpdateTime = get<int64_t>();
                    auto version = get<int64_t>();
                    auto ttl = get<int64_t>();
                    auto maxIdle = get<int64_t>();
                    // skip bytes in initial frame
                    rd_ptr(static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS - 10 * INT64_SIZE);

                    auto key = get<serialization::pimpl::data>();
                    auto value = get<serialization::pimpl::data>();

                    fast_forward_to_end_frame();

                    return {std::move(key), std::move(value), cost, creationTime, expirationTime, hits, lastAccessTime,
                            lastStoredTime, lastUpdateTime, version, ttl, maxIdle};
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, typename std::pair<typename T::first_type, typename T::second_type>>::value, T>::type
                inline get() {
                    auto key = get<typename T::first_type>();
                    auto value = get<typename T::second_type>();
                    return {std::move(key), std::move(value)};
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, cp::raft_group_id>::value, T>::type
                get();

                template<typename T>
                typename std::enable_if<std::is_same<T, typename boost::optional<typename std::remove_reference<typename std::remove_cv<typename T::value_type>::type>::type>>::value, T>::type
                inline get() {
                    typedef typename std::remove_reference<typename std::remove_cv<typename T::value_type>::type>::type type;
                    return get_nullable<type>();
                }

                template<typename T>
                typename std::enable_if<std::is_same<T, codec::ErrorHolder>::value, T>::type
                inline get() {
                    // skip begin frame
                    rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);

                    auto f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    auto error_code = get<int32_t>();
                    // skip bytes in initial frame
                    rd_ptr(static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS - INT32_SIZE);

                    auto class_name = get<std::string>();
                    auto message = get_nullable<std::string>();
                    auto stack_traces = get<std::vector<codec::StackTraceElement>>();
                    codec::ErrorHolder h = {error_code, std::move(class_name), std::move(message),
                                            std::move(stack_traces)};

                    fast_forward_to_end_frame();

                    return h;
                }

                template<typename T>
                boost::optional<T> get_nullable() {
                    if (next_frame_is_null_frame()) {
                        // skip next frame with null flag
                        rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS);
                        return boost::none;
                    }
                    return boost::make_optional(get<T>());
                }

                template<typename T>
                T get_first_fixed_sized_field() {
                    assert(buffer_index_ == 0 && offset_ == 0);
                    // skip header
                    rd_ptr(RESPONSE_HEADER_LEN);
                    return get<T>();
                }

                inline boost::uuids::uuid get_first_uuid() {
                    assert(buffer_index_ == 0 && offset_ == 0);
                    // skip header
                    rd_ptr(RESPONSE_HEADER_LEN);
                    return get<boost::uuids::uuid>();
                }

                template<typename T>
                boost::optional<T> get_first_var_sized_field() {
                    assert(buffer_index_ == 0 && offset_ == 0);
                    skip_frame();
                    return get<T>();
                }

                template<typename T>
                boost::optional<T> get_first_optional_var_sized_field() {
                    assert(buffer_index_ == 0 && offset_ == 0);
                    skip_frame();
                    return get_nullable<T>();
                }
                //----- Getter methods end --------------------------

                //---------------------- Setters -------------------------------
                inline void set(uint8_t value) {
                    *wr_ptr(UINT8_SIZE) = value;
                }

                inline void set(int8_t value) {
                    *wr_ptr(INT8_SIZE) = value;
                }

                inline void set(bool value) {
                    *wr_ptr(UINT8_SIZE) = value ? 1 : 0;
                }

                inline void set(char value) {
                    *wr_ptr(UINT8_SIZE) = value;
                }

                inline void set(uint16_t value) {
                    boost::endian::endian_store<boost::uint16_t, 2, boost::endian::order::little>(wr_ptr(UINT16_SIZE),
                                                                                                  value);
                }

                inline void set(int16_t value) {
                    boost::endian::endian_store<boost::int16_t, 2, boost::endian::order::little>(wr_ptr(INT16_SIZE),
                                                                                                 value);
                }

                inline void set(uint32_t value) {
                    boost::endian::endian_store<boost::uint32_t, 4, boost::endian::order::little>(wr_ptr(UINT32_SIZE),
                                                                                                  value);
                }

                inline void set(int32_t value) {
                    boost::endian::endian_store<boost::int32_t, 4, boost::endian::order::little>(wr_ptr(INT32_SIZE),
                                                                                                 value);
                }

                inline void set(uint64_t value) {
                    boost::endian::endian_store<boost::uint64_t, 8, boost::endian::order::little>(wr_ptr(UINT64_SIZE),
                                                                                                  value);
                }

                inline void set(int64_t value) {
                    boost::endian::endian_store<boost::int64_t, 8, boost::endian::order::little>(wr_ptr(INT64_SIZE),
                                                                                                 value);
                }

                void set_message_type(int32_t type);

                void set_correlation_id(int64_t id);

                /**
                 * @return the number of acks will be send for a request
                 */
                int8_t get_number_of_backups() const;

                void set_partition_id(int32_t partition_id);

                template<typename T>
                void set_nullable(const T *value, bool is_final = false) {
                    bool isNull = (NULL == value);
                    if (isNull) {
                        auto *h = reinterpret_cast<frame_header_t *>(wr_ptr(sizeof(frame_header_t)));
                        *h = null_frame();
                        if (is_final) {
                            h->flags |= IS_FINAL_FLAG;
                        }
                    } else {
                        set(*value, is_final);
                    }
                }

                inline void set(const std::string &value, bool is_final = false) {
                    auto h = reinterpret_cast<frame_header_t *>(wr_ptr(sizeof(frame_header_t)));
                    auto len = value.length();
                    h->frame_len = sizeof(frame_header_t) + len;
                    if (is_final) {
                        h->flags |= IS_FINAL_FLAG;
                    }

                    memcpy(wr_ptr(len), &value[0], len);
                }

                inline void set(const std::string *value) {
                    set_nullable<std::string>(value);
                }

                inline void set(const address &a, bool is_final = false) {
                    add_begin_frame();

                    auto f = reinterpret_cast<frame_header_t *>(wr_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    f->frame_len = SIZE_OF_FRAME_LENGTH_AND_FLAGS + INT32_SIZE;
                    f->flags = DEFAULT_FLAGS;
                    set(static_cast<int32_t>(a.get_port()));

                    set(a.get_host());

                    add_end_frame(is_final);
                }

                void set(const codec::holder::paging_predicate_holder &p, bool is_final = false);
                void set(const query::anchor_data_list &list, bool is_final = false);

                inline void set(const config::index_config &c, bool is_final = false) {
                    add_begin_frame();

                    auto f = reinterpret_cast<frame_header_t *>(wr_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    f->frame_len = SIZE_OF_FRAME_LENGTH_AND_FLAGS + INT32_SIZE;
                    f->flags = DEFAULT_FLAGS;
                    set(static_cast<int32_t>(c.type));

                    set(c.name);
                    set(c.attributes);
                    set(c.options);

                    add_end_frame(is_final);
                }

                inline void set(const config::index_config::bitmap_index_options &o, bool is_final = false) {
                    add_begin_frame();

                    auto f = reinterpret_cast<frame_header_t *>(wr_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    f->frame_len = SIZE_OF_FRAME_LENGTH_AND_FLAGS + INT32_SIZE;
                    f->flags = DEFAULT_FLAGS;
                    set(static_cast<int32_t>(o.transformation));

                    set(o.key);

                    add_end_frame(is_final);
                }

                inline void set(boost::uuids::uuid uuid) {
                    auto nil = uuid.is_nil();
                    set(nil);
                    if (!nil) {
                        boost::endian::endian_reverse_inplace<int64_t>(*reinterpret_cast<int64_t *>(uuid.data));
                        boost::endian::endian_reverse_inplace<int64_t>(*reinterpret_cast<int64_t *>(uuid.data + util::Bits::LONG_SIZE_IN_BYTES));
                        std::memcpy(wr_ptr(sizeof(boost::uuids::uuid)), uuid.data, sizeof(boost::uuids::uuid));
                    } else {
                        wr_ptr(sizeof(boost::uuids::uuid));
                    }
                }

                inline void set(const serialization::pimpl::data &value, bool is_final = false) {
                    if (value.data_size() == 0) {
                        auto *h = reinterpret_cast<frame_header_t *>(wr_ptr(sizeof(frame_header_t)));
                        *h = null_frame();
                        if (is_final) {
                            h->flags |= IS_FINAL_FLAG;
                        }
                        return;
                    }
                    auto &bytes = value.to_byte_array();
                    auto frame_length = sizeof(frame_header_t) + bytes.size();
                    auto fp = wr_ptr(frame_length);
                    auto *header = reinterpret_cast<frame_header_t *>(fp);
                    header->frame_len = frame_length;
                    header->flags  = is_final ? IS_FINAL_FLAG : DEFAULT_FLAGS;
                    std::memcpy(fp + SIZE_OF_FRAME_LENGTH_AND_FLAGS, &bytes[0], bytes.size());
                }

                inline void set(const serialization::pimpl::data *value, bool is_final = false) {
                    set_nullable<serialization::pimpl::data>(value, is_final);
                }

                void set(const cp::raft_group_id &o, bool is_final = false);

                template<typename T>
                typename std::enable_if<std::is_same<T, typename boost::optional<typename std::remove_reference<typename std::remove_cv<typename T::value_type>::type>::type>>::value, void>::type
                inline set(const T &value, bool is_final = false) {
                    typedef typename std::remove_reference<typename std::remove_cv<typename T::value_type>::type>::type type;
                    return set_nullable<type>(value.get_ptr(), is_final);
                }

                void set(unsigned char *memory, boost::uuids::uuid uuid);

                template<typename K, typename V>
                void set(const std::pair<K, V> &entry) {
                    set(entry.first);
                    set(entry.second);
                }

                template<typename T>
                void set(const std::vector<T> &values, bool is_final = false) {
                    auto *h = reinterpret_cast<frame_header_t *>(wr_ptr(sizeof(frame_header_t)));
                    *h = begin_frame();

                    for (auto &item : values) {
                        set(item);
                    }

                    h = reinterpret_cast<frame_header_t *>(wr_ptr(sizeof(frame_header_t)));
                    *h = end_frame();
                    if (is_final) {
                        h->flags |= IS_FINAL_FLAG;
                    }
                }
                //----- Setter methods end ---------------------

                //----- utility methods -------------------
                /**
                * Tries to read enough bytes to fill the message from the provided ByteBuffer
                */
                void fill_message_from(util::ByteBuffer &buffer, bool &is_final, size_t &remaining_bytes_in_frame);

                size_t size() const;

                int32_t get_message_type() const;

                uint16_t get_header_flags() const;

                void set_header_flags(uint16_t new_flags);

                void inline add_flag(uint16_t flag) {
                    set_header_flags(get_header_flags() | flag);
                }

                int64_t get_correlation_id() const;

                int32_t get_partition_id() const;

                inline bool is_flag_set(uint16_t flag_mask) const {
                    return flag_mask == (get_header_flags() & flag_mask);
                }

                static inline bool is_flag_set(uint16_t flags, uint16_t flag_mask) {
                    return flag_mask == (flags & flag_mask);
                }

                //Builder function
                void append(std::shared_ptr<ClientMessage> msg);

                bool is_retryable() const;

                void set_retryable(bool should_retry);

                std::string get_operation_name() const;

                void set_operation_name(const std::string &name);

                inline void skip_frame() {
                    auto *f = reinterpret_cast<frame_header_t *>(rd_ptr(SIZE_OF_FRAME_LENGTH_AND_FLAGS));
                    rd_ptr(static_cast<int32_t>(f->frame_len) - SIZE_OF_FRAME_LENGTH_AND_FLAGS);
                }

                void fast_forward_to_end_frame();

                static const frame_header_t &null_frame();
                static const frame_header_t &begin_frame();
                static const frame_header_t &end_frame();

                void drop_fragmentation_frame();

                friend std::ostream HAZELCAST_API &operator<<(std::ostream &os, const ClientMessage &message);

            private:
                static const frame_header_t NULL_FRAME;
                static const frame_header_t BEGIN_FRAME;
                static const frame_header_t END_FRAME;

                template<typename T>
                void set_primitive_vector(const std::vector<T> &values, bool is_final = false) {
                    int32_t len = SIZE_OF_FRAME_LENGTH_AND_FLAGS + values.size() * sizeof(T);
                    auto memory = wr_ptr(len);
                    auto *h = reinterpret_cast<frame_header_t *>(memory);
                    h->frame_len = len;
                    h->flags  = is_final ? IS_FINAL_FLAG : DEFAULT_FLAGS;

                    memory += SIZE_OF_FRAME_LENGTH_AND_FLAGS;
                    for (auto v : values) {
                        boost::endian::endian_store<T, sizeof(T), boost::endian::order::little>( memory, v );
                        memory += sizeof(T);
                    }
                }

                inline bool next_frame_is_data_structure_end_frame() {
                    return is_flag_set(
                            boost::endian::endian_load<boost::uint16_t, 2, boost::endian::order::little>(
                                    peek(SIZE_OF_FRAME_LENGTH_AND_FLAGS) + FLAGS_FIELD_OFFSET),
                            END_DATA_STRUCTURE_FLAG);
                }

                inline bool next_frame_is_null_frame() {
                    return is_flag_set(
                            boost::endian::endian_load<boost::uint16_t, 2, boost::endian::order::little>(
                                    peek(SIZE_OF_FRAME_LENGTH_AND_FLAGS) + FLAGS_FIELD_OFFSET),
                            IS_NULL_FLAG);
                }

                boost::uuids::uuid get_uuid() {
                    boost::uuids::uuid u;
                    memcpy(&u.data, rd_ptr(sizeof(boost::uuids::uuid)), sizeof(boost::uuids::uuid));
                    boost::endian::endian_reverse_inplace<int64_t>(*reinterpret_cast<int64_t *>(u.data));
                    boost::endian::endian_reverse_inplace<int64_t>(*reinterpret_cast<int64_t *>(u.data + util::Bits::LONG_SIZE_IN_BYTES));
                    return u;
                }

                void add_begin_frame() {
                    auto *f = reinterpret_cast<frame_header_t *>(wr_ptr(sizeof(frame_header_t)));
                    *f = begin_frame();
                }

                void add_end_frame(bool is_final) {
                    auto ef = reinterpret_cast<frame_header_t *>(wr_ptr(sizeof(frame_header_t)));
                    *ef = end_frame();
                    if (is_final) {
                        ef->flags |= IS_FINAL_FLAG;
                    }
                }

                template<typename T>
                typename std::enable_if<(std::is_same<int32_t, T>::value || std::is_same<int64_t, T>::value), size_t>::type
                static constexpr get_sizeof() {
                    return sizeof(T);
                }

                template<typename T>
                typename std::enable_if<std::is_same<boost::uuids::uuid, T>::value, size_t>::type
                static constexpr get_sizeof() {
                    return 17;
                }

                bool retryable_;
                std::string operation_name_;

                std::vector<std::vector<byte>> data_buffer_;
                size_t buffer_index_;
                size_t offset_;
            };

            template<>
            void HAZELCAST_API ClientMessage::set(const std::vector<byte> &values, bool is_final);

            template<>
            void HAZELCAST_API ClientMessage::set(const std::vector<int32_t> &values, bool is_final);

            template<>
            void HAZELCAST_API ClientMessage::set(const std::vector<int64_t> &values, bool is_final);

            template<>
            void HAZELCAST_API ClientMessage::set(const std::vector<boost::uuids::uuid> &values, bool is_final);

            template<>
            void HAZELCAST_API ClientMessage::set(const std::vector<std::pair<boost::uuids::uuid, int64_t>> &values, bool is_final);

        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
