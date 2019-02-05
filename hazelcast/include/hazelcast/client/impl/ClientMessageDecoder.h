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

#ifndef HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_
#define HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_

#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/client/serialization/pimpl/SerializationService.h"

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

                virtual boost::shared_ptr<T>
                decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService) = 0;
            };

            template<typename CODEC, typename V>
            class DataMessageDecoder : public ClientMessageDecoder<V> {
            public:
                virtual boost::shared_ptr<V>
                decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService) {
                    return boost::shared_ptr<V>(serializationService.toObject<V>(
                            CODEC::ResponseParameters::decode(*clientMessage).response.get()));
                }

                static const boost::shared_ptr<ClientMessageDecoder<V> > &instance();

            private:
                static const boost::shared_ptr<ClientMessageDecoder<V> > singleton;
            };

            template<typename CODEC, typename V>
            const boost::shared_ptr<ClientMessageDecoder<V> > DataMessageDecoder<CODEC, V>::singleton(
                    new DataMessageDecoder<CODEC, V>());

            template<typename CODEC, typename V>
            const boost::shared_ptr<ClientMessageDecoder<V> > &DataMessageDecoder<CODEC, V>::instance() {
                return singleton;
            }

            class HAZELCAST_API VoidMessageDecoder : public ClientMessageDecoder<void> {
            public:
                virtual boost::shared_ptr<void>
                decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService);

                static const boost::shared_ptr<ClientMessageDecoder<void> > &instance();

            private:
                static const boost::shared_ptr<ClientMessageDecoder<void> > singleton;
            };


            template<typename CODEC, typename PRIMITIVE_TYPE>
            class PrimitiveMessageDecoder : public ClientMessageDecoder<PRIMITIVE_TYPE> {
            public:
                virtual boost::shared_ptr<PRIMITIVE_TYPE>
                decodeClientMessage(const boost::shared_ptr<protocol::ClientMessage> &clientMessage,
                                    serialization::pimpl::SerializationService &serializationService) {
                    return boost::shared_ptr<PRIMITIVE_TYPE>(new PRIMITIVE_TYPE(
                            CODEC::ResponseParameters::decode(*clientMessage).response));
                }

                static const boost::shared_ptr<ClientMessageDecoder<PRIMITIVE_TYPE> > &instance();

            private:
                static const boost::shared_ptr<ClientMessageDecoder<PRIMITIVE_TYPE> > singleton;
            };

            template<typename CODEC, typename PRIMITIVE_TYPE>
            const boost::shared_ptr<ClientMessageDecoder<PRIMITIVE_TYPE> > PrimitiveMessageDecoder<CODEC, PRIMITIVE_TYPE>::singleton(
                    new PrimitiveMessageDecoder<CODEC, PRIMITIVE_TYPE>());

            template<typename CODEC, typename PRIMITIVE_TYPE>
            const boost::shared_ptr<ClientMessageDecoder<PRIMITIVE_TYPE> > &
            PrimitiveMessageDecoder<CODEC, PRIMITIVE_TYPE>::instance() {
                return singleton;
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_IMPL_CLIENTMESSAGEDECODER_H_

