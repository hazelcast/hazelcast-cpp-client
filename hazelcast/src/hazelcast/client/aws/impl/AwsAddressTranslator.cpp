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

#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/client/aws/impl/AwsAddressTranslator.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/Address.h"
#include "hazelcast/client/exception/IException.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                AwsAddressTranslator::AwsAddressTranslator(config::ClientAwsConfig &awsConfig) {
                    if (awsConfig.isEnabled() && !awsConfig.isInsideAws()) {
                        awsClient = std::auto_ptr<AWSClient>(new AWSClient(awsConfig));
                    }
                }

                Address AwsAddressTranslator::translate(const Address &address) {
                    // if no translation is needed just return the address as it is
                    if (NULL == awsClient.get()) {
                        return address;
                    }

                    Address translatedAddress = address;

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    refresh();

                    if (findFromCache(address, translatedAddress)) {
                        return translatedAddress;
                    }

                    std::stringstream out;
                    out << "No translation is found for private ip:" << address;
                    throw exception::IOException("AwsAddressTranslator::translate", out.str());
                }

                void AwsAddressTranslator::refresh() {
                    try {
                        privateToPublic = boost::shared_ptr<std::map<std::string, std::string> >(
                                new std::map<std::string, std::string>(awsClient->getAddresses()));
                    } catch (exception::IException &e) {
                        util::ILogger::getLogger().warning(std::string("AWS addresses failed to load: ") + e.what());
                    }
                }

                bool AwsAddressTranslator::findFromCache(const Address &address, Address &translatedAddress) {
                    boost::shared_ptr<std::map<std::string, std::string> > mapping = privateToPublic;
                    if (mapping.get() == NULL) {
                        return false;
                    }

                    std::map<std::string, std::string>::const_iterator publicAddressIt = mapping->find(
                            address.getHost());
                    if (publicAddressIt != mapping->end()) {
                        const std::string &publicIp = (*publicAddressIt).second;
                        if (!publicIp.empty()) {
                            translatedAddress = Address((*publicAddressIt).second, address.getPort());
                            return true;
                        }
                    }

                    return false;
                }
            }
        }
    }
}

