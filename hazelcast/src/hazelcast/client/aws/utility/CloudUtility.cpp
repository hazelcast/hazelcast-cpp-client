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
                                                                               const config::ClientAwsConfig &awsConfig) {
                    std::map<std::string, std::string> privatePublicPairs;

                    pt::ptree tree;
                    pt::read_xml(stream, tree);

                    // Use get_child to find the node containing the reservation set, and iterate over
                    // its children.
                    BOOST_FOREACH(pt::ptree::value_type & item,
                                  tree.get_child("DescribeInstancesResponse.reservationSet")) {
                        BOOST_FOREACH(pt::ptree::value_type & instanceItem,
                                      item.second.get_child("instancesSet")) {
                            std::string state = instanceItem.second.get<std::string>(
                                    "instanceState.name");
                            boost::optional<std::string> privateIp = instanceItem.second.get_optional<std::string>(
                                    "privateIpAddress");
                            boost::optional<std::string> publicIp = instanceItem.second.get_optional<std::string>(
                                    "ipAddress");
                            boost::optional<std::string> instanceName = instanceItem.second.get_optional<std::string>(
                                    "tagset.item.value");

                            std::string instName = instanceName.value_or("");
                            std::string prIp = privateIp.value_or("");
                            std::string pubIp = publicIp.value_or("");

                            if (privateIp) {
                                if (state != "running") {
                                    std::ostringstream out;
                                    out << "Ignoring EC2 instance [" << instName << "][" <<
                                    prIp <<
                                    "] reason: the instance is not running but " << state;
                                    util::ILogger::getLogger().finest(out.str());
                                } else if (!acceptTag(awsConfig, instanceItem.second)) {
                                    std::ostringstream out;
                                    out << "Ignoring EC2 instance [" << instName << "][" <<
                                    prIp <<
                                    "] reason: tag-key/tag-value don't match.";
                                    util::ILogger::getLogger().finest(out.str());
                                } else if (!acceptGroupName(awsConfig, instanceItem.second)) {
                                    std::ostringstream out;
                                    out << "Ignoring EC2 instance [" << instName << "][" <<
                                    prIp <<
                                    "] reason: security-group-name doesn't match.";
                                    util::ILogger::getLogger().finest(out.str());
                                } else {
                                    privatePublicPairs[prIp] = pubIp;
                                    std::ostringstream out;
                                    out << "Accepting EC2 instance [" << instName << "][" <<
                                    prIp << "]";
                                    util::ILogger::getLogger().finest(out.str());
                                }
                            }
                        }
                    }
                    return privatePublicPairs;
                }

                bool CloudUtility::acceptTag(const config::ClientAwsConfig &awsConfig, pt::ptree &reservationSetItem) {
                    const std::string &expectedTagKey = awsConfig.getTagKey();
                    const std::string &expectedTagValue = awsConfig.getTagValue();

                    if (expectedTagKey.empty()) {
                        return true;
                    }

                    bool emptyExpectedValue = expectedTagValue.empty();

                    boost::optional<pt::ptree &> tags = reservationSetItem.get_child_optional("tagSet");
                    if (tags) {
                        BOOST_FOREACH(pt::ptree::value_type &item, tags.get()) {
                                        std::string key = item.second.get_optional<std::string>("key").get_value_or("");
                                        std::string value = item.second.get_optional<std::string>("value").get_value_or(
                                                "");
                                        if (key == expectedTagKey &&
                                            (emptyExpectedValue || value == expectedTagValue)) {
                                            return true;
                                        }
                        }
                    }
                    return false;
                }

                bool CloudUtility::acceptGroupName(const config::ClientAwsConfig &awsConfig, pt::ptree &reservationSetItem) {
                    const std::string &securityGroupName = awsConfig.getSecurityGroupName();
                    if (securityGroupName.empty()) {
                        return true;
                    }

                    boost::optional<pt::ptree &> groups = reservationSetItem.get_child_optional("groupSet");
                    if (groups) {
                        BOOST_FOREACH(pt::ptree::value_type &item, groups.get()) {
                                        boost::optional<std::string> groupNameOptional = item.second.get_optional<std::string>(
                                                "groupName");
                                        if (groupNameOptional && groupNameOptional.get() == securityGroupName) {
                                            return true;
                                        }
                        }
                    }

                    return false;
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

