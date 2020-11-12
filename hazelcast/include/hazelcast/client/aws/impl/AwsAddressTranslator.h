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

#include <memory>
#include <unordered_map>
#include <memory>

#include "hazelcast/client/aws/aws_client.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/connection/AddressTranslator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    class logger;

    namespace client {
        namespace aws {
            namespace impl {
                class HAZELCAST_API AwsAddressTranslator : public connection::AddressTranslator {
                public:
                    AwsAddressTranslator(config::client_aws_config &aws_config, logger &lg);

                    /**
                     * Translates an IP address from the private AWS network to the public network.
                     *
                     * @param address the private address to translate
                     * @return public address of network whose private address is given.
                     *
                     * @throws io if the address can not be translated.
                     */
                    address translate(const address &addr) override;

                    /**
                     * Update the internal lookup table from AWS.
                     */
                    void refresh() override;

                private:
                    bool find_from_cache(const address &addr, address &translated_address);

                    std::unique_ptr<aws_client> aws_client_;
                    util::Sync<std::shared_ptr<std::unordered_map<std::string, std::string> > > private_to_public_;
                    logger &logger_;
                };
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif


