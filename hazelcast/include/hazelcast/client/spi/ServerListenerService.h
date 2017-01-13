/*
 * Copyright (c) 2008-2017, Hazelcast, Inc. All Rights Reserved.
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
//
// Created by sancar koyunlu on 6/24/13.




#ifndef HAZELCAST_SERVER_LISTENER_SERVICE
#define HAZELCAST_SERVER_LISTENER_SERVICE

#include "hazelcast/client/connection/CallFuture.h"
#include "hazelcast/client/protocol/codec/IRemoveListenerCodec.h"
#include "hazelcast/client/protocol/ClientMessage.h"
#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace impl {
            class ClientRequest;

            class BaseRemoveListenerRequest;

            class BaseEventHandler;
        }

        namespace connection{
            class CallPromise;
        }

        namespace serialization {
            namespace pimpl {
                class Data;
            }

        }

        namespace protocol {
            class ClientMessage;
            namespace codec {
                class IAddListenerCodec;
                class IRemoveListenerCodec;
            }
        }

        namespace spi {
            class ClientContext;

            namespace impl {
                namespace listener {
                    class EventRegistration;
                }
            }

            class HAZELCAST_API ServerListenerService {
            public:
                ServerListenerService(spi::ClientContext &clientContext);

                std::string registerListener(std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
                                             int partitionId, client::impl::BaseEventHandler *handler);

                std::string registerListener(std::auto_ptr<protocol::codec::IAddListenerCodec> addListenerCodec,
                                             client::impl::BaseEventHandler *handler);

                void reRegisterListener(std::string registrationId, protocol::ClientMessage *response);

                bool deRegisterListener(protocol::codec::IRemoveListenerCodec &removeListenerCodec);

                void retryFailedListener(boost::shared_ptr<connection::CallPromise> listenerPromise);

                void triggerFailedListeners();

            private:

                util::Mutex failedListenerLock;
                std::vector< boost::shared_ptr<connection::CallPromise> > failedListeners;

                util::SynchronizedMap<std::string, impl::listener::EventRegistration> registrationIdMap;
                util::SynchronizedMap<std::string, const std::string > registrationAliasMap;
                spi::ClientContext &clientContext;

                std::string registerInternal(std::auto_ptr<protocol::codec::IAddListenerCodec> &addListenerCodec,
                                             hazelcast::client::connection::CallFuture &future);
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //__ServerListenerService_H_

