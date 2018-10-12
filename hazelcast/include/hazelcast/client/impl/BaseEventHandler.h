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
//
// Created by sancar koyunlu on 14/01/14.
//

#ifndef HAZELCAST_BaseEventHandler
#define HAZELCAST_BaseEventHandler

#include "hazelcast/util/HazelcastDll.h"
#include <string>
#include <memory>
#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/util/ILogger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace protocol {
            class ClientMessage;
        }
        namespace impl {
            class HAZELCAST_API BaseEventHandler : public spi::EventHandler<protocol::ClientMessage> {
            public:
                BaseEventHandler();

                virtual ~BaseEventHandler();
                
                virtual void handle(std::auto_ptr<protocol::ClientMessage> message) = 0;

                // TODO: Remove the above method after changing and regenerating the codecs
                virtual void handle(const boost::shared_ptr<protocol::ClientMessage> &event);

                std::string registrationId;

                /**
                 *  This method is called before registration request is sent to node.
                 *
                 *  Note that this method will also be called while first registered node is dead
                 *  and re-registering to a second node.
                 */
                virtual void beforeListenerRegister() {
                }

                /**
                 *  This method is called when registration request response is successfully returned from node.
                 *
                 *  Note that this method will also be called while first registered node is dead
                 *  and re-registering to a second node.
                 */
                virtual void onListenerRegister() {
                }

                void setLogger(util::ILogger *logger);

                util::ILogger *getLogger() const;

            protected:
                util::ILogger *logger;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_BaseEventHandler

