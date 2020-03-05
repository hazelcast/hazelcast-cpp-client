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

#ifndef HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_
#define HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_

#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            /**
             * Interface to pass specific decode methods to generic data structures that is dealing with client message
             */
            template<typename T>
            class ClientMessageDecoder {
            public:
                virtual ~ClientMessageDecoder() {
                }

                virtual std::shared_ptr<T>
                decodeClientMessage(const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService) = 0;
            };

            template<typename CODEC, typename V>
            class DataMessageDecoder : public ClientMessageDecoder<V> {
            public:
                virtual std::shared_ptr<V>
                decodeClientMessage(const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService) {
                    return std::shared_ptr<V>(serializationService.toObject<V>(
                            CODEC::ResponseParameters::decode(*clientMessage).response.get()));
                }

                static const std::shared_ptr<ClientMessageDecoder<V> > &instance();
            };

            template<typename CODEC, typename V>
            const std::shared_ptr<ClientMessageDecoder<V> > &DataMessageDecoder<CODEC, V>::instance() {
                static std::shared_ptr<ClientMessageDecoder<V> > singleton(new DataMessageDecoder<CODEC, V>());
                return singleton;
            }

            class HAZELCAST_API VoidMessageDecoder : public ClientMessageDecoder<void> {
            public:
                virtual std::shared_ptr<void>
                decodeClientMessage(const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService);

                static const std::shared_ptr<ClientMessageDecoder<void> > &instance();
            };


            template<typename CODEC, typename PRIMITIVE_TYPE>
            class PrimitiveMessageDecoder : public ClientMessageDecoder<PRIMITIVE_TYPE> {
            public:
                virtual std::shared_ptr<PRIMITIVE_TYPE>
                decodeClientMessage(const std::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService) {
                    return std::shared_ptr<PRIMITIVE_TYPE>(new PRIMITIVE_TYPE(
                            CODEC::ResponseParameters::decode(*clientMessage).response));
                }

                static const std::shared_ptr<ClientMessageDecoder<PRIMITIVE_TYPE> > &instance();
            };

            template<typename CODEC, typename PRIMITIVE_TYPE>
            const std::shared_ptr<ClientMessageDecoder<PRIMITIVE_TYPE> > &
            PrimitiveMessageDecoder<CODEC, PRIMITIVE_TYPE>::instance() {
                static std::shared_ptr<ClientMessageDecoder<PRIMITIVE_TYPE> > singleton(
                        new PrimitiveMessageDecoder<CODEC, PRIMITIVE_TYPE>());
                return singleton;
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_

