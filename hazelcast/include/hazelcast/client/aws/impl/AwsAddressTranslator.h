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
#include <map>
#include <memory>

#include "hazelcast/client/aws/AWSClient.h"
#include "hazelcast/util/Sync.h"
#include "hazelcast/client/connection/AddressTranslator.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        namespace aws {
            namespace impl {
                class HAZELCAST_API AwsAddressTranslator : public connection::AddressTranslator {
                public:
                    AwsAddressTranslator(config::ClientAwsConfig &awsConfig, util::ILogger &logger);

                    /**
                     * Translates an IP address from the private AWS network to the public network.
                     *
                     * @param address the private address to translate
                     * @return public address of network whose private address is given.
                     *
                     * @throws IOException if the address can not be translated.
                     */
                    Address translate(const Address &address);

                    /**
                     * Update the internal lookup table from AWS.
                     */
                    void refresh();

                private:
                    bool findFromCache(const Address &address, Address &translatedAddress);

                    std::unique_ptr<AWSClient> awsClient;
                    util::Sync<std::shared_ptr<std::map<std::string, std::string> > > privateToPublic;
                    util::ILogger &logger;
                };
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

