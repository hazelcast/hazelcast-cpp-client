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

#include <unordered_map>

#include "hazelcast/client/aws/aws_client.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/util/export.h"
#include "hazelcast/client/connection/AddressProvider.h"
#include "hazelcast/logger.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace config {
            class client_network_config;
        }
        namespace spi {
            namespace impl {
                class HAZELCAST_API AwsAddressProvider : public connection::AddressProvider {
                public:
                    AwsAddressProvider(config::client_aws_config &aws_config, int aws_member_port, logger &lg);

                    ~AwsAddressProvider() override;

                    std::vector<address> load_addresses() override;

                private:
                    std::string aws_member_port_;
                    logger &logger_;
                    aws::aws_client aws_client_;
                    util::Sync<std::unordered_map<std::string, std::string> > private_to_public_;

                    void update_lookup_table();

                    std::unordered_map<std::string, std::string> get_lookup_table();
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


