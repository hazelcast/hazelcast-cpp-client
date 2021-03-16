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

#include "hazelcast/client/config/cloud_config.h"
#include "hazelcast/util/export.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {
                namespace discovery {
                    class HAZELCAST_API cloud_discovery {
                    public:
                        static constexpr const char *CLOUD_SERVER = "coordinator.hazelcast.cloud";
                        static constexpr const char *CLOUD_URL_PATH = "/cluster/discovery?token=";
                        static constexpr const char *PRIVATE_ADDRESS_PROPERTY = "private-address";
                        static constexpr const char *PUBLIC_ADDRESS_PROPERTY = "public-address";

                        cloud_discovery(config::cloud_config &config, std::chrono::steady_clock::duration timeout);

                        std::unordered_map<address, address> get_addresses();

                        // This method is public for testing
                        static std::unordered_map<address, address> parse_json_response(std::istream &conn_stream);

                    private:
                        config::cloud_config &cloud_config_;
                        std::chrono::steady_clock::duration timeout_;

                        static address create_address(const std::string &hostname, int default_port);
                    };
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


