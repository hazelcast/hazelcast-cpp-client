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

#include "hazelcast/client/connection/ConnectionListener.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            /**
             * Provides connection listen capabilities.
             */
            class HAZELCAST_API ConnectionListenable {
            public:
                /**
                 * Registers a ConnectionListener.
                 *
                 * If the same listener is registered multiple times, it will be notified multiple times.
                 *
                 * @param listener the ConnectionListener to add.
                 * @throws null_pointer if listener is null.
                 */
                virtual void add_connection_listener(const std::shared_ptr<ConnectionListener> &listener) = 0;
            };
        }
    }
}



