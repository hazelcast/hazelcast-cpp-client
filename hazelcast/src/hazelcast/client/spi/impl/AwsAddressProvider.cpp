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

#include <boost/foreach.hpp>

#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/client/spi/impl/AwsAddressProvider.h"
#include "hazelcast/client/config/ClientNetworkConfig.h"
#include "hazelcast/util/AddressHelper.h"

namespace hazelcast {
    namespace client {
        namespace spi {
            namespace impl {

                AwsAddressProvider::AwsAddressProvider(config::ClientAwsConfig &awsConfig, int awsMemberPort,
                                                       util::ILogger &logger) : awsMemberPort(
                        util::IOUtil::to_string<int>(awsMemberPort)), logger(logger), awsClient(awsConfig) {
                }

                std::vector<Address> AwsAddressProvider::loadAddresses() {
                    updateLookupTable();
                    std::map<std::string, std::string> lookupTable = getLookupTable();
                    std::vector<Address> addresses;

                    typedef std::map<std::string, std::string> LookupTable;
                    BOOST_FOREACH(const LookupTable::value_type &privateAddress, lookupTable) {
                                    std::vector<Address> possibleAddresses = util::AddressHelper::getSocketAddresses(
                                            privateAddress.first + ":" + awsMemberPort);
                                    addresses.insert(addresses.begin(), possibleAddresses.begin(),
                                                     possibleAddresses.end());
                                }
                    return addresses;
                }

                void AwsAddressProvider::updateLookupTable() {
                    try {
                        privateToPublic = awsClient.getAddresses();
                    } catch (exception::IException &e) {
                        logger.warning() << "Aws addresses failed to load: " << e.getMessage();
                    }
                }

                std::map<std::string, std::string> AwsAddressProvider::getLookupTable() {
                    return privateToPublic;
                }

                AwsAddressProvider::~AwsAddressProvider() {
                }
            }
        }
    }
}
