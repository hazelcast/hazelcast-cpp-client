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


#pragma once

#include <string>
#include <memory>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/spi/EventHandler.h"
#include "hazelcast/logger.h"

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

                ~BaseEventHandler() override;

                std::string registration_id;

                /**
                 *  This method is called before registration request is sent to node.
                 *
                 *  Note that this method will also be called while first registered node is dead
                 *  and re-registering to a second node.
                 */
                void before_listener_register() override {
                }

                /**
                 *  This method is called when registration request response is successfully returned from node.
                 *
                 *  Note that this method will also be called while first registered node is dead
                 *  and re-registering to a second node.
                 */
                void on_listener_register() override {
                }

                void set_logger(logger *lg);

                logger *get_logger() const;

            protected:
                logger *logger_;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



