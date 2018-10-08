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
#ifndef HAZELCAST_CLIENT_AWS_UTILITY_CLOUDUTILITY_H_
#define HAZELCAST_CLIENT_AWS_UTILITY_CLOUDUTILITY_H_

#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif

namespace pt = boost::property_tree;

namespace hazelcast {
    namespace util {
        class ILogger;
    }
    namespace client {
        namespace config {
            class ClientAwsConfig;
        }
        namespace aws {
            namespace utility {
                class HAZELCAST_API CloudUtility {
                public:
                    /**
                     * Unmarshal the response from {@link DescribeInstances} and return the discovered node map.
                     * The map contains mappings from private to public IP.
                     * If there is an exception while unmarshalling the response, returns an empty map.
                     *
                     * @param stream    the response XML stream
                     * @param logger the logger to be used for logging any warnings during unmarshal.
                     * @return map from private to public IP or empty map in case of exceptions
                     */
                    static std::map<std::string, std::string> unmarshalTheResponse(std::istream &stream,
                            util::ILogger &logger);

                    static void unmarshalJsonResponse(std::istream &stream, config::ClientAwsConfig &awsConfig,
                                                      std::map<std::string, std::string> &attributes);
                };
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif /* HAZELCAST_CLIENT_AWS_UTILITY_CLOUDUTILITY_H_ */
