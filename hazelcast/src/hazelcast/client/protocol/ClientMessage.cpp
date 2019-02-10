/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
 * ClientMessage.cpp
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan
 */

#include <assert.h>

#include "hazelcast/client/protocol/ClientMessage.h"
#include <hazelcast/client/protocol/codec/UUIDCodec.h>
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/protocol/codec/AddressCodec.h"
#include "hazelcast/client/protocol/codec/MemberCodec.h"
#include "hazelcast/client/protocol/codec/DataEntryViewCodec.h"
#include "hazelcast/client/serialization/pimpl/Data.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/client/protocol/codec/StackTraceElementCodec.h"
#include "hazelcast/client/Member.h"
#include "hazelcast/client/map/DataEntryView.h"
#include "hazelcast/client/protocol/codec/StackTraceElement.h"

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

            std::auto_ptr<ClientMessage> ClientMessage::createForEncode(int32_t size) {
                std::auto_ptr<ClientMessage> msg(new ClientMessage(size));
                msg->wrapForEncode(size);
                return msg;
            }

            std::auto_ptr<ClientMessage> ClientMessage::create(int32_t size) {
                return std::auto_ptr<ClientMessage>(new ClientMessage(size));
            }

            //----- Setter methods begin --------------------------------------
            void ClientMessage::setFrameLength(int32_t length) {
                util::Bits::nativeToLittleEndian4(&length, &buffer[FRAME_LENGTH_FIELD_OFFSET]);
            }

            void ClientMessage::setMessageType(uint16_t type) {
                util::Bits::nativeToLittleEndian2(&type, &buffer[TYPE_FIELD_OFFSET]);
            }

            void ClientMessage::setVersion(uint8_t value) {
                buffer[VERSION_FIELD_OFFSET] = value;
            }

            uint8_t ClientMessage::getFlags() {
                return buffer[FLAGS_FIELD_OFFSET];
            }

            void ClientMessage::addFlag(uint8_t flags) {
                buffer[FLAGS_FIELD_OFFSET] = getFlags() | flags;
            }

            void ClientMessage::setCorrelationId(int64_t id) {
                util::Bits::nativeToLittleEndian8(&id, &buffer[CORRELATION_ID_FIELD_OFFSET]);
            }

            void ClientMessage::setPartitionId(int32_t partitionId) {
                util::Bits::nativeToLittleEndian4(&partitionId, &buffer[PARTITION_ID_FIELD_OFFSET]);
            }

            void ClientMessage::setDataOffset(uint16_t offset) {
                util::Bits::nativeToLittleEndian2(&offset, &buffer[DATA_OFFSET_FIELD_OFFSET]);
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
                size_t numRead = byteBuff.readBytes(&buffer[offset], numToRead);

                if (numRead == numToRead) {
                    wrapForRead(frameLen, ClientMessage::HEADER_SIZE);
                }

                return (int32_t) numRead;
            }

            //----- Getter methods begin -------------------
            int32_t ClientMessage::getFrameLength() const {
                int32_t result;

                util::Bits::littleEndianToNative4(
                        &buffer[FRAME_LENGTH_FIELD_OFFSET], &result);

                return result;
            }

            uint16_t ClientMessage::getMessageType() const {
                uint16_t type;

                util::Bits::littleEndianToNative2(&buffer[TYPE_FIELD_OFFSET], &type);

                return type;
            }

            uint8_t ClientMessage::getVersion() {
                return buffer[VERSION_FIELD_OFFSET];
            }

            int64_t ClientMessage::getCorrelationId() const {
                int64_t value;
                util::Bits::littleEndianToNative8(&buffer[CORRELATION_ID_FIELD_OFFSET], &value);
                return value;
            }

            int32_t ClientMessage::getPartitionId() const {
                int32_t value;
                util::Bits::littleEndianToNative4(&buffer[PARTITION_ID_FIELD_OFFSET], &value);
                return value;
            }

            uint16_t ClientMessage::getDataOffset() const {
                uint16_t value;
                util::Bits::littleEndianToNative2(&buffer[DATA_OFFSET_FIELD_OFFSET], &value);
                return value;
            }

            bool ClientMessage::isFlagSet(uint8_t flag) const {
                return flag == (buffer[FLAGS_FIELD_OFFSET] & flag);
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

                checkAvailable(len);

                byte *start = ix();
                std::auto_ptr<std::vector<byte> > bytes = std::auto_ptr<std::vector<byte> >(new std::vector<byte>(start,
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
            std::pair<Address, std::vector<int64_t> > ClientMessage::get() {
                Address address = codec::AddressCodec::decode(*this);
                std::vector<int64_t> values = getArray<int64_t>();
                return std::make_pair(address, values);
            }

            template<>
            std::pair<Address, std::vector<int32_t> > ClientMessage::get() {
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
                memcpy(&buffer[existingFrameLen], &msg->buffer[0], (size_t) dataSize);
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

                    buffer.resize(newSize, 0);
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

            int32_t ClientMessage::writeTo(Socket &socket, int32_t offset, int32_t frameLen) {
                int32_t numBytesSent = 0;

                int32_t numBytesLeft = frameLen - offset;
                if (numBytesLeft > 0) {
                    numBytesSent = socket.send(&buffer[offset], numBytesLeft);
                }

                return numBytesSent;
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
        }
    }
}
