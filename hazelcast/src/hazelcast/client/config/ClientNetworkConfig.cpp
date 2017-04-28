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

#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/util/Preconditions.h"

namespace hazelcast {
    namespace client {
        namespace config {
            ClientNetworkConfig::ClientNetworkConfig()
                    : connectionTimeout(5000) {
            }

            SSLConfig &ClientNetworkConfig::getSSLConfig() {
                util::Preconditions::checkSSL("getSSLConfig");
                return sslConfig;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSSLConfig(const config::SSLConfig &sslConfig) {
                util::Preconditions::checkSSL("setSSLConfig");
                this->sslConfig = sslConfig;
                return *this;
            }

            int64_t ClientNetworkConfig::getConnectionTimeout() const {
                return connectionTimeout;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionTimeout(int64_t connectionTimeoutInMillis) {
                this->connectionTimeout = connectionTimeoutInMillis;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAwsConfig(const ClientAwsConfig &clientAwsConfig) {
                util::Preconditions::checkSSL("setAwsConfig");
                this->clientAwsConfig = clientAwsConfig;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::getAwsConfig() {
                util::Preconditions::checkSSL("getAwsConfig");
                return clientAwsConfig;
            }
        }
    }
}
