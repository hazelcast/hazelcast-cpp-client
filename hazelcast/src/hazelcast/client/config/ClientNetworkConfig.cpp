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

#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"

namespace hazelcast {
    namespace client {
        namespace config {
            int32_t ClientNetworkConfig::CONNECTION_ATTEMPT_PERIOD = 3000;

            ClientNetworkConfig::ClientNetworkConfig()
                    : connectionTimeout(5000), smartRouting(true), connectionAttemptLimit(-1),
                      connectionAttemptPeriod(CONNECTION_ATTEMPT_PERIOD) {
            }

            SSLConfig &ClientNetworkConfig::getSSLConfig() {
                return sslConfig;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSSLConfig(const config::SSLConfig &sslConfig) {
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
                this->clientAwsConfig = clientAwsConfig;
                return *this;
            }

            ClientAwsConfig &ClientNetworkConfig::getAwsConfig() {
                return clientAwsConfig;
            }

            bool ClientNetworkConfig::isSmartRouting() const {
                return smartRouting;
            }

            ClientNetworkConfig &ClientNetworkConfig::setSmartRouting(bool smartRouting) {
                ClientNetworkConfig::smartRouting = smartRouting;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptLimit() const {
                return connectionAttemptLimit;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptLimit(int32_t connectionAttemptLimit) {
                if (connectionAttemptLimit < 0) {
                    throw exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptLimit",
                                                              "connectionAttemptLimit cannot be negative");
                }
                this->connectionAttemptLimit = connectionAttemptLimit;
                return *this;
            }

            int32_t ClientNetworkConfig::getConnectionAttemptPeriod() const {
                return connectionAttemptPeriod;
            }

            ClientNetworkConfig &ClientNetworkConfig::setConnectionAttemptPeriod(int32_t connectionAttemptPeriod) {
                if (connectionAttemptPeriod < 0) {
                    throw exception::IllegalArgumentException("ClientNetworkConfig::setConnectionAttemptPeriod",
                                                              "connectionAttemptPeriod cannot be negative");
                }
                this->connectionAttemptPeriod = connectionAttemptPeriod;
                return *this;
            }

            std::vector<Address> ClientNetworkConfig::getAddresses() const {
                return addressList;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddresses(const std::vector<Address> &addresses) {
                addressList.insert(addressList.end(), addresses.begin(), addresses.end());
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::setAddresses(const std::vector<Address> &addresses) {
                addressList = addresses;
                return *this;
            }

            ClientNetworkConfig &ClientNetworkConfig::addAddress(const Address &address) {
                addressList.push_back(address);
                return *this;
            }
        }
    }
}
