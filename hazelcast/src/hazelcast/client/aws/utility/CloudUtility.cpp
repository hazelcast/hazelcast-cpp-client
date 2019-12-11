/*
 * Copyright (c) 2008-2019, Hazelcast, Inc. All Rights Reserved.
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

#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/util/ILogger.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace utility {
                std::map<std::string, std::string> CloudUtility::unmarshalTheResponse(std::istream &stream,
                        util::ILogger &logger) {
                    std::map<std::string, std::string> privatePublicPairs;

                    pt::ptree tree;
                    try {
                        pt::read_xml(stream, tree);
                    } catch (pt::xml_parser_error &e) {
                        logger.warning(
                                std::string("The parsed xml stream has errors: ") + e.what());
                        return privatePublicPairs;
                    }

                    // Use get_child to find the node containing the reservation set, and iterate over
                    // its children.
                    BOOST_FOREACH(pt::ptree::value_type & item,
                                  tree.get_child("DescribeInstancesResponse.reservationSet")) {
                                    BOOST_FOREACH(pt::ptree::value_type & instanceItem,
                                                  item.second.get_child("instancesSet")) {
                                                    boost::optional<std::string> privateIp = instanceItem.second.get_optional<std::string>(
                                                            "privateIpAddress");
                                                    boost::optional<std::string> publicIp = instanceItem.second.get_optional<std::string>(
                                                            "ipAddress");
                                                    std::string prIp = privateIp.value_or("");
                                                    std::string pubIp = publicIp.value_or("");

                                                    if (privateIp) {
                                                        privatePublicPairs[prIp] = pubIp;
                                                        if (logger.isFinestEnabled()) {
                                                            boost::optional<std::string> instanceName = instanceItem.second.get_optional<std::string>(
                                                                    "tagset.item.value");

                                                            std::string instName = instanceName.value_or("");

                                                            logger.finest(
                                                                    std::string("Accepting EC2 instance [") + instName +
                                                                    "][" + prIp + "]");
                                                        }
                                                    }
                                                }
                                }
                    return privatePublicPairs;
                }

                void CloudUtility::unmarshalJsonResponse(std::istream &stream, config::ClientAwsConfig &awsConfig,
                                                         std::map<std::string, std::string> &attributes) {
                    pt::ptree json;
                    pt::read_json(stream, json);
                    awsConfig.setAccessKey(json.get_optional<std::string>("AccessKeyId").get_value_or(""));
                    awsConfig.setSecretKey(json.get_optional<std::string>("SecretAccessKey").get_value_or(""));
                    attributes["X-Amz-Security-Token"] = json.get_optional<std::string>("Token").get_value_or("");
                }
            }
        }
    }
}

