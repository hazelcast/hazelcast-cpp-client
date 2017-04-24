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
                AwsAddressTranslator::AwsAddressTranslator(const config::ClientAwsConfig &awsConfig) {
                    if (awsConfig.isEnabled() && !awsConfig.isInsideAws()) {
                        awsClient = std::auto_ptr<AWSClient>(new AWSClient(awsConfig));
                    }
                }

                std::auto_ptr<Address> AwsAddressTranslator::translate(const Address &address) {
                    // if no translation is needed just return the address as it is
                    if (NULL == awsClient.get()) {
                        return std::auto_ptr<Address>(new Address(address));
                    }

                    std::auto_ptr<Address> result = findFromCache(address);
                    if (NULL != result.get()) {
                        return result;
                    }

                    refresh();

                    result = findFromCache(address);
                    if (NULL != result.get()) {
                        return result;
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

                std::auto_ptr<Address> AwsAddressTranslator::findFromCache(const Address &address) {
                    boost::shared_ptr<std::map<std::string, std::string> > mapping = privateToPublic;
                    if (mapping.get() == NULL) {
                        return std::auto_ptr<Address>();
                    }

                    std::map<std::string, std::string>::const_iterator publicAddressIt = mapping->find(
                            address.getHost());
                    if (publicAddressIt != mapping->end()) {
                        const std::string &publicIp = (*publicAddressIt).second;
                        if (!publicIp.empty()) {
                            return std::auto_ptr<Address>(new Address((*publicAddressIt).second, address.getPort()));
                        }
                    }

                    return std::auto_ptr<Address>();
                }
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL

