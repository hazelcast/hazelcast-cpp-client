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
#ifndef HAZELCAST_CLIENT_AWS_IMPL_AWSADDRESSTRANSLATOR_H_
#define HAZELCAST_CLIENT_AWS_IMPL_AWSADDRESSTRANSLATOR_H_

#ifdef HZ_BUILD_WITH_SSL

#include <memory>
#include <map>
#include <boost/shared_ptr.hpp>

#include "hazelcast/client/aws/AWSClient.h"
#include "hazelcast/util/Atomic.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace hazelcast {
    namespace client {
        class Address;

        namespace aws {
            namespace impl {
                class HAZELCAST_API AwsAddressTranslator {
                public:
                    /**
                     * AwsAddressTranslator loads EC2 IP addresses with given AWS credentials.
                     *
                     * Keeps a lookup table of private to public IP addresses.
                     */
                    AwsAddressTranslator(const config::ClientAwsConfig &awsConfig);

                    /**
                     * Translates an IP address from the private AWS network to the public network.
                     *
                     * @param address the private address to translate
                     * @return public address of network whose private address is given.
                     *
                     * @throws IOException if the address can not be translated.
                     */
                    virtual std::auto_ptr<Address> translate(const Address &address);

                private:
                    /**
                     * Update the internal lookup table from AWS.
                     */
                    virtual void refresh();

                    std::auto_ptr<Address> findFromCache(const Address &address);

                    std::auto_ptr<AWSClient> awsClient;
                    util::Atomic<boost::shared_ptr<std::map<std::string, std::string> > > privateToPublic;
                };
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif // HZ_BUILD_WITH_SSL

#endif /* HAZELCAST_CLIENT_AWS_IMPL_AWSADDRESSTRANSLATOR_H_ */
