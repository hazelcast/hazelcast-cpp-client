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
// Created by sancar koyunlu on 25/12/13.
//

#ifndef HAZELCAST_IListener
#define HAZELCAST_IListener

#include "hazelcast/client/connection/IOSelector.h"
#include "hazelcast/util/SocketSet.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class HAZELCAST_API InSelector : public IOSelector {
            public:
                InSelector(ClientConnectionManagerImpl &connectionManager, const config::SocketOptions &socketOptions);

                void listenInternal();

                bool start();

                virtual const std::string getName() const;
            };
        }
    }
}

#endif //HAZELCAST_IListener

