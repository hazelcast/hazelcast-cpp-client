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

#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time.hpp>

#include <asio.hpp>

#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/aws/impl/DescribeInstances.h"
#include "hazelcast/client/aws/impl/Constants.h"
#include "hazelcast/client/aws/security/EC2RequestSigner.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"
#include "hazelcast/client/config/ClientAwsConfig.h"

namespace hazelcast {
    namespace client {
        namespace aws {
            namespace impl {
                const std::string DescribeInstances::QUERY_PREFIX = "/?";

                DescribeInstances::DescribeInstances(config::ClientAwsConfig &awsConfig,
                                                     const std::string &endpoint) : awsConfig(awsConfig),
                                                                                    endpoint(endpoint) {
/*TODO
                    checkKeysFromIamRoles();
*/

                    std::string timeStamp = getFormattedTimestamp();
                    rs = std::auto_ptr<security::EC2RequestSigner>(
                            new security::EC2RequestSigner(awsConfig, timeStamp, endpoint));
                    attributes["Action"] = "DescribeInstances";
                    attributes["Version"] = impl::Constants::DOC_VERSION;
                    attributes["X-Amz-Algorithm"] = impl::Constants::SIGNATURE_METHOD_V4;
                    attributes["X-Amz-Credential"] = rs->createFormattedCredential();
                    attributes["X-Amz-Date"] = timeStamp;
                    attributes["X-Amz-SignedHeaders"] = "host";
                    attributes["X-Amz-Expires"] = "30";
                }

                DescribeInstances::~DescribeInstances() {
                }

                std::map<std::string, std::string> DescribeInstances::execute() {
                    std::string signature = rs->sign(attributes);
                    attributes["X-Amz-Signature"] = signature;

                    std::istream &stream = callService();
                    return utility::CloudUtility::unmarshalTheResponse(stream, awsConfig);
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
                    httpsClient = std::auto_ptr<util::SyncHttpsClient>(new util::SyncHttpsClient(endpoint.c_str(), QUERY_PREFIX + query));
                    return httpsClient->openConnection();
                }
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL

