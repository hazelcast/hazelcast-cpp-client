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
 *
 *  Created on: Mar 17, 2015
 *      Author: ihsan demir
 */

/**
* <p>
* Client Message is the carrier framed data as defined below.
* </p>
* <p>
* Any request parameter, response or event data will be carried in
* the payload.
* </p>
* <p/>
* <pre>
* 0                   1                   2                   3
* 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
* +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
* |R|                      Frame Length                           |
* +-------------+---------------+---------------------------------+
* |  Version    |B|E|  Flags  |L|               Type              |
* +-------------+---------------+---------------------------------+
* |                       CorrelationId                           |
* |                                                               |
* +---------------------------------------------------------------+
* |R|                      PartitionId                            |
* +-----------------------------+---------------------------------+
* |        Data Offset          |                                 |
* +-----------------------------+                                 |
* |                      Message Payload Data                    ...
* |                                                              ...
*
* </pre>
*/

#ifndef HAZELCAST_CLIENT_CLIENTMESSAGE_H_
#define HAZELCAST_CLIENT_CLIENTMESSAGE_H_

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

#include <string>
#include <memory>
#include <vector>
#include <assert.h>
#include <map>
#include <ostream>

#include "hazelcast/util/LittleEndianBufferWrapper.h"
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace util {
        class ByteBuffer;
        class UUID;
    }

    namespace client {
        class Address;

        class Member;

        class Socket;

        namespace map {
            class DataEntryView;
        }

        namespace serialization {
            namespace pimpl {
                class Data;
            }
        }

        namespace impl {
            class DistributedObjectInfo;
        }

        namespace protocol {
            namespace codec {
                class StackTraceElement;
            }

            class HAZELCAST_API ClientTypes {
            public:
                static const std::string CPP;
            };

			class HAZELCAST_API ClientMessage : public util::LittleEndianBufferWrapper {
            public:
                /**
                * Current protocol version
                */
                static const uint8_t HAZELCAST_CLIENT_PROTOCOL_VERSION = 1;

                /**
                * Begin Flag
                */
                static const uint8_t BEGIN_FLAG = 0x80;
                /**
                * End Flag
                */
                static const uint8_t END_FLAG = 0x40;
                /**
                * Begin and End Flags
                */
                static const uint8_t BEGIN_AND_END_FLAGS = (BEGIN_FLAG | END_FLAG);

                /**
                * Listener Event Flag
                */
                static const uint8_t LISTENER_EVENT_FLAG = 0x01;

                static const int32_t FRAME_LENGTH_FIELD_OFFSET = 0;
                static const int32_t VERSION_FIELD_OFFSET = FRAME_LENGTH_FIELD_OFFSET + INT32_SIZE;
                static const int32_t FLAGS_FIELD_OFFSET = VERSION_FIELD_OFFSET + UINT8_SIZE;
                static const int32_t TYPE_FIELD_OFFSET = FLAGS_FIELD_OFFSET + UINT8_SIZE;
                static const int32_t CORRELATION_ID_FIELD_OFFSET = TYPE_FIELD_OFFSET + UINT16_SIZE;
                static const int32_t PARTITION_ID_FIELD_OFFSET = CORRELATION_ID_FIELD_OFFSET + INT64_SIZE;
                static const int32_t DATA_OFFSET_FIELD_OFFSET = PARTITION_ID_FIELD_OFFSET + INT32_SIZE;

                /**
                * ClientMessage Fixed Header size in bytes
                */
                static const uint16_t HEADER_SIZE = DATA_OFFSET_FIELD_OFFSET + UINT16_SIZE;

                virtual ~ClientMessage();

                static std::auto_ptr<ClientMessage> createForEncode(int32_t size);

                static std::auto_ptr<ClientMessage> create(int32_t size);

                //----- Setter methods begin --------------------------------------
                // bring base class set methods into the derived class
                using util::LittleEndianBufferWrapper::set;

                void setFrameLength(int32_t length);

                void setMessageType(uint16_t type);

                void setVersion(uint8_t value);

                uint8_t getFlags();

                void addFlag(uint8_t flags);

                void setCorrelationId(int64_t id);

                void setPartitionId(int32_t partitionId);

                void setDataOffset(uint16_t offset);

                void updateFrameLength();

                template<typename T>
                void setNullable(const T *value) {
                    bool isNull = (NULL == value);
                    set(isNull);
                    if (!isNull) {
                        set(*value);
                    }
                }

                void set(const std::string *data);

                void set(const Address &data);

                void set(const serialization::pimpl::Data &data);

                void set(const serialization::pimpl::Data *data);

                template<typename K, typename V>
                void set(const std::pair<K, V> &entry) {
                    set(entry.first);
                    set(entry.second);
                }

                template<typename T>
                void setArray(const std::vector<T> &values) {
                    int32_t len = (int32_t) values.size();
                    set(len);

                    if (len > 0) {
                        for (typename std::vector<T>::const_iterator it = values.begin(); it != values.end(); ++it) {
                            set(*it);
                        }
                    }
                }

                template<typename T>
                void setArray(const std::vector<T> *value) {
                    bool isNull = (NULL == value);
                    set(isNull);
                    if (!isNull) {
                        setArray<T>(*value);
                    }
                }
                //----- Setter methods end ---------------------

                /**
                * Tries to read enough bytes to fill the message from the provided ByteBuffer
                */
                int32_t fillMessageFrom(util::ByteBuffer &buffer, int32_t numBytesToRead, int32_t frameLen);

                //----- Getter methods begin -------------------
                int32_t getFrameLength() const;

                uint16_t getMessageType() const;

                uint8_t getVersion();

                int64_t getCorrelationId() const;

                int32_t getPartitionId() const;

                uint16_t getDataOffset() const;

                bool isFlagSet(uint8_t flag) const;

                //-----Getters that change the index position---------
                template<typename T>
                T get() {
                    assert(0);
                }

                template<typename KEY, typename VALUE>
                std::pair<KEY, VALUE> getEntry() {
                    KEY key = get<KEY>();
                    VALUE val = get<VALUE>();
                    return std::pair<KEY, VALUE>(key, val);
                }

                template<typename T>
                std::auto_ptr<T> getNullable() {
                    std::auto_ptr<T> result;
                    if (getBoolean()) {
                        return result;
                    }
                    return std::auto_ptr<T>(new T(get<T>()));
                }

                template<typename T>
                std::vector<T> getArray() {
                    int32_t len = getInt32();

                    std::vector<T> result(len);
                    for (int i = 0; i < len; ++i) {
                        result[i] = get<T>();
                    }
                    return result;
                }

                template<typename T>
                std::auto_ptr<std::vector<T> > getNullableArray() {
                    std::auto_ptr<std::vector<T> > result;
                    if (getBoolean()) {
                        return result;
                    }
                    return std::auto_ptr<std::vector<T> >(new std::vector<T>(getArray<T>()));
                }

                template<typename KEY, typename VALUE>
                std::vector<std::pair<KEY, VALUE> > getEntryArray() {
                    int32_t len = getInt32();

                    std::vector<std::pair<KEY, VALUE> > result(len);
                    for (int i = 0; i < len; ++i) {
                        result[i] = getEntry<KEY, VALUE>();
                    }
                    return result;
                }

                template<typename KEY, typename VALUE>
                std::auto_ptr<std::vector<std::pair<KEY, VALUE> > > getNullableEntryArray() {
                    std::auto_ptr<std::vector<std::pair<KEY, VALUE> > > result;
                    if (getBoolean()) {
                        return result;
                    }

                    return std::auto_ptr<std::vector<std::pair<KEY, VALUE> > >(
                            new std::vector<std::pair<KEY, VALUE> >(getEntryArray<KEY, VALUE>()));
                }
                //----- Getter methods end --------------------------

                //----- Data size calculation functions BEGIN -------
                static int32_t calculateDataSize(uint8_t param);

                static int32_t calculateDataSize(bool param);

                static int32_t calculateDataSize(int32_t param);

                static int32_t calculateDataSize(int64_t param);


                template<typename T>
                static int32_t calculateDataSizeNullable(const T *param) {
                    int32_t size = INT8_SIZE;
                    if (NULL != param) {
                        size += calculateDataSize(*param);
                    }
                    return size;
                }

                static int32_t calculateDataSize(const std::string &param);

                static int32_t calculateDataSize(const std::string *param);

                static int32_t calculateDataSize(const Address &param);

                static int32_t calculateDataSize(const serialization::pimpl::Data &param);

                static int32_t calculateDataSize(const serialization::pimpl::Data *param);

                template<typename K, typename V>
                static int32_t calculateDataSize(const std::pair<K, V> &param) {
                    return calculateDataSize(param.first) + calculateDataSize(param.second);
                }

                template<typename T>
                static int32_t calculateDataSize(const std::vector<T> &param) {
                    int32_t dataSize = INT32_SIZE;
                    for (typename std::vector<T>::const_iterator it = param.begin(); param.end() != it; ++it) {
                        dataSize += calculateDataSize(*it);
                    }
                    return dataSize;
                }

                template<typename T>
                static int32_t calculateDataSize(const std::vector<T> *param) {
                    int32_t size = INT8_SIZE;
                    if (NULL != param) {
                        size += calculateDataSize<T>(*param);
                    }
                    return size;
                }

                template<typename KEY, typename VALUE>
                static int32_t calculateDataSize(const std::vector<std::pair<KEY, VALUE> > &param) {
                    int32_t size = INT32_SIZE;
                    for (typename std::vector<std::pair<KEY, VALUE> >::const_iterator it = param.begin();
                         param.end() != it; ++it) {
                        size += calculateDataSize<KEY, VALUE>(*it);
                    }
                    return size;
                }

                template<typename KEY, typename VALUE>
                static int32_t calculateDataSize(const std::vector<std::pair<KEY, VALUE> > *param) {
                    int32_t size = INT8_SIZE;

                    if (NULL != param) {
                        size += calculateDataSize<KEY, VALUE>(*param);
                    }

                    return size;
                }

                //----- Data size calculation functions END ---------

                //Builder function
                void append(const ClientMessage *msg);

                int32_t getDataSize() const;

                bool isRetryable() const;

                void setRetryable(bool shouldRetry);

                /**
                 * Returns the number of bytes sent on the socket
                 **/
                int32_t writeTo(Socket &socket, int32_t offset, int32_t frameLen);

                /**
                 * Checks the frame size and total data size to validate the message size.
                 *
                 * @return true if the message is constructed.
                 */
                bool isComplete() const;

                friend std::ostream &operator<<(std::ostream &os, const ClientMessage &message);

            private:
                ClientMessage(int32_t size);

                inline void wrapForEncode(int32_t size);

                void ensureBufferSize(int32_t newCapacity);

                int32_t findSuitableCapacity(int32_t requiredCapacity, int32_t existingCapacity) const;

                bool retryable;
            };

            template<>
            uint8_t ClientMessage::get();

            template<>
            bool ClientMessage::get();

            template<>
            int8_t ClientMessage::get();

            template<>
            int16_t ClientMessage::get();

            template<>
            uint16_t ClientMessage::get();

            template<>
            int32_t ClientMessage::get();

            template<>
            uint32_t ClientMessage::get();

            template<>
            int64_t ClientMessage::get();

            template<>
            uint64_t ClientMessage::get();

            template<>
            std::string ClientMessage::get();

            template<>
            Address ClientMessage::get();

            template<>
            util::UUID ClientMessage::get();

            template<>
            Member ClientMessage::get();

            template<>
            map::DataEntryView ClientMessage::get();

            template<>
            serialization::pimpl::Data ClientMessage::get();

            template<>
            impl::DistributedObjectInfo ClientMessage::get();

            template<>
            codec::StackTraceElement ClientMessage::get();

            template<>
            std::vector<int32_t> ClientMessage::get();

            template<>
            std::pair<serialization::pimpl::Data, serialization::pimpl::Data> ClientMessage::get();

            template<>
            std::pair<Address, std::vector<int32_t> > ClientMessage::get();

            template<>
            std::pair<Address, std::vector<int64_t> > ClientMessage::get();

            template<>
            std::pair<std::string, int64_t> ClientMessage::get();

        }

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_CLIENTMESSAGE_H_ */
