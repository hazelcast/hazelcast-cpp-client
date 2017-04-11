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
// Created by sancar koyunlu on 28/12/13.
//

#ifndef HAZELCAST_IOHandler
#define HAZELCAST_IOHandler

#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/util/HazelcastDll.h"

#include <string>

namespace hazelcast {
    namespace client {
        namespace connection {
            class IOSelector;

            class Connection;

            class HAZELCAST_API IOHandler : public ListenerTask {
            public:

                IOHandler(Connection &connection, IOSelector & ioSelector);

                virtual ~IOHandler();

                virtual void handle() = 0;

                void registerSocket();

                /* should be called from registered io thread */
                void deRegisterSocket();

            protected:
                IOSelector & ioSelector;
                Connection &connection;

                void registerHandler();

                void handleSocketException(const std::string& message);
            };
        }
    }
}

#endif //HAZELCAST_IOHandler

