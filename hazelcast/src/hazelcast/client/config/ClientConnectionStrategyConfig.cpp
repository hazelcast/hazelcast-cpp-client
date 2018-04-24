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
#include "hazelcast/client/config/ClientConnectionStrategyConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {
            ClientConnectionStrategyConfig::ClientConnectionStrategyConfig() : asyncStart(false), reconnectMode(ON) {
            }

            ClientConnectionStrategyConfig::ReconnectMode ClientConnectionStrategyConfig::getReconnectMode() const {
                return reconnectMode;
            }

            bool ClientConnectionStrategyConfig::isAsyncStart() const {
                return asyncStart;
            }

            ClientConnectionStrategyConfig &ClientConnectionStrategyConfig::setAsyncStart(bool asyncStart) {
                this->asyncStart = asyncStart;
                return *this;
            }

            ClientConnectionStrategyConfig &
            ClientConnectionStrategyConfig::setReconnectMode(ReconnectMode reconnectMode) {
                this->reconnectMode = reconnectMode;
                return *this;
            }

        }
    }
}

