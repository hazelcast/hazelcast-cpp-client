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
#ifdef HZ_BUILD_WITH_SSL

#include <boost/algorithm/string/replace.hpp>

#include "hazelcast/client/aws/AWSClient.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/exception/IllegalArgumentException.h"
#include "hazelcast/client/config/ClientAwsConfig.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            AWSClient::AWSClient(config::ClientAwsConfig &awsConfig) : awsConfig(awsConfig) {
                this->endpoint = awsConfig.getHostHeader();
                if (!awsConfig.getRegion().empty() && awsConfig.getRegion().length() > 0) {
                    if (awsConfig.getHostHeader().find("ec2.") != 0) {
                        throw exception::InvalidConfigurationException("AWSClient::AWSClient",
                                                                       "HostHeader should start with \"ec2.\" prefix");
                    }
                    boost::replace_all(this->endpoint, "ec2.", std::string("ec2.") + awsConfig.getRegion() + ".");
                }
            }

            std::map<std::string, std::string> AWSClient::getAddresses() {
                return impl::DescribeInstances(awsConfig, endpoint).execute();
            }

            const std::string &AWSClient::getEndpoint() const {
                return endpoint;
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL

