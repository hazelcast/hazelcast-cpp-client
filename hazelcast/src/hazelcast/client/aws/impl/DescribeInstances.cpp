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

#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time.hpp>

#include <asio.hpp>

#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/aws/impl/Filter.h"
#include "hazelcast/client/aws/impl/Constants.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/exception/IOException.h"
#include "hazelcast/util/SyncHttpClient.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                const std::string DescribeInstances::QUERY_PREFIX = "/?";
                const std::string DescribeInstances::IAM_ROLE_ENDPOINT = "169.254.169.254";
                const std::string DescribeInstances::IAM_ROLE_QUERY = "/latest/meta-data/iam/security-credentials/";
                const std::string DescribeInstances::IAM_TASK_ROLE_ENDPOINT = "169.254.170.2";

                DescribeInstances::DescribeInstances(config::ClientAwsConfig &awsConfig, const std::string &endpoint,
                                                     util::ILogger &logger) : awsConfig(awsConfig), endpoint(endpoint),
                                                                              logger(logger) {
                    checkKeysFromIamRoles();

                    std::string timeStamp = getFormattedTimestamp();
                    rs = std::unique_ptr<security::EC2RequestSigner>(
                            new security::EC2RequestSigner(awsConfig, timeStamp, endpoint));
                    attributes["Action"] = "DescribeInstances";
                    attributes["Version"] = impl::Constants::DOC_VERSION;
                    attributes["X-Amz-Algorithm"] = impl::Constants::SIGNATURE_METHOD_V4;
                    attributes["X-Amz-Credential"] = rs->createFormattedCredential();
                    attributes["X-Amz-Date"] = timeStamp;
                    attributes["X-Amz-SignedHeaders"] = "host";
                    attributes["X-Amz-Expires"] = "30";
                    addFilters();
                }

                DescribeInstances::~DescribeInstances() {
                }

                std::map<std::string, std::string> DescribeInstances::execute() {
                    std::string signature = rs->sign(attributes);
                    attributes["X-Amz-Signature"] = signature;

                    std::istream &stream = callService();
                    return utility::CloudUtility::unmarshalTheResponse(stream, logger);
                }

                std::string DescribeInstances::getFormattedTimestamp() {
                    using namespace boost::posix_time;
                    ptime now = second_clock::universal_time();

                    std::ostringstream out;
                    std::locale timeLocale(out.getloc(), new time_facet(impl::Constants::DATE_FORMAT));
                    out.imbue(timeLocale);
                    out << now;
                    return out.str();
                }

                std::istream &DescribeInstances::callService() {
                    std::string query = rs->getCanonicalizedQueryString(attributes);
                    httpsClient = std::unique_ptr<util::SyncHttpsClient>(
                            new util::SyncHttpsClient(endpoint.c_str(), QUERY_PREFIX + query));
                    return httpsClient->openConnection();
                }

                void DescribeInstances::checkKeysFromIamRoles() {
                    if (awsConfig.getAccessKey().empty() || !awsConfig.getIamRole().empty()) {
                        tryGetDefaultIamRole();
                        if (!awsConfig.getIamRole().empty()) {
                            getKeysFromIamRole();
                        } else {
                            getKeysFromIamTaskRole();
                        }
                    }
                }

                void DescribeInstances::tryGetDefaultIamRole() {
                    // if none of the below are true
                    if (!(awsConfig.getIamRole().empty() || awsConfig.getIamRole() == "DEFAULT")) {
                        // stop here. No point looking up the default role.
                        return;
                    }
                    try {
                        util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, IAM_ROLE_QUERY);
                        std::string roleName;
                        std::istream &responseStream = httpClient.openConnection();
                        responseStream >> roleName;
                        awsConfig.setIamRole(roleName);
                    } catch (exception::IOException &e) {
                        throw exception::InvalidConfigurationException("tryGetDefaultIamRole",
                                                                       std::string("Invalid Aws Configuration") +
                                                                       e.what());
                    }
                }

                void DescribeInstances::getKeysFromIamTaskRole() {
                    // before giving up, attempt to discover whether we're running in an ECS Container,
                    // in which case, AWS_CONTAINER_CREDENTIALS_RELATIVE_URI will exist as an env var.
                    const char *uri = getenv(Constants::ECS_CREDENTIALS_ENV_VAR_NAME);
                    if (!uri) {
                        throw exception::IllegalArgumentException("getKeysFromIamTaskRole",
                                                                  "Could not acquire credentials! Did not find declared AWS access key or IAM Role, and could not discover IAM Task Role or default role.");
                    }
                    
                    util::SyncHttpClient httpClient(IAM_TASK_ROLE_ENDPOINT, uri);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << uri << ". \n " << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamTaskRole", out.str());
                    }
                }

                void DescribeInstances::getKeysFromIamRole() {
                    std::string query = "/latest/meta-data/iam/security-credentials/" + awsConfig.getIamRole();

                    util::SyncHttpClient httpClient(IAM_ROLE_ENDPOINT, query);

                    try {
                        std::istream &istream = httpClient.openConnection();
                        parseAndStoreRoleCreds(istream);
                    } catch (exception::IException &e) {
                        std::stringstream out;
                        out << "Unable to retrieve credentials from IAM Task Role. URI: " << query << ". \n " << e.what();
                        throw exception::InvalidConfigurationException("getKeysFromIamRole", out.str());
                    }
                }

                void DescribeInstances::parseAndStoreRoleCreds(std::istream &in) {
                    utility::CloudUtility::unmarshalJsonResponse(in, awsConfig, attributes);
                }

                /**
                 * Add available filters to narrow down the scope of the query
                 */
                void DescribeInstances::addFilters() {
                    Filter filter;
                    if (!awsConfig.getTagKey().empty()) {
                        if (!awsConfig.getTagValue().empty()) {
                            filter.addFilter(std::string("tag:") + awsConfig.getTagKey(), awsConfig.getTagValue());
                        } else {
                            filter.addFilter("tag-key", awsConfig.getTagKey());
                        }
                    } else if (!awsConfig.getTagValue().empty())  {
                        filter.addFilter("tag-value", awsConfig.getTagValue());
                    }

                    if (!awsConfig.getSecurityGroupName().empty()) {
                        filter.addFilter("instance.group-name", awsConfig.getSecurityGroupName());
                    }

                    filter.addFilter("instance-state-name", "running");
                    const std::map<std::string, std::string> &filters = filter.getFilters();
                    attributes.insert(filters.begin(), filters.end());
                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

