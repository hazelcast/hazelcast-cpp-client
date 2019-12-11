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

#ifdef HZ_BUILD_WITH_SSL

#include <fstream>
#include <asio.hpp>

#include "ClientTestSupport.h"
#include "hazelcast/client/config/ClientAwsConfig.h"
#include "hazelcast/client/aws/utility/CloudUtility.h"

namespace awsutil = hazelcast::client::aws::utility;

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class CloudUtilityTest : public ClientTestSupport {
                };

                TEST_F (CloudUtilityTest, testUnmarshallResponseXml) {
                    std::filebuf fb;
                    ASSERT_TRUE(fb.open("hazelcast/test/resources/sample_aws_response.xml", std::ios::in));
                    std::istream responseStream(&fb);

                    config::ClientAwsConfig awsConfig;
                    std::map<std::string, std::string> results = awsutil::CloudUtility::unmarshalTheResponse(
                            responseStream, getLogger());
                    ASSERT_EQ(4U, results.size());
                    ASSERT_NE(results.end(), results.find("10.0.16.13"));
                    ASSERT_EQ("", results["10.0.16.13"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.17"));
                    ASSERT_EQ("54.85.192.215", results["10.0.16.17"]);
                    ASSERT_NE(results.end(), results.find("10.0.16.25"));
                    ASSERT_EQ("", results["10.0.16.25"]);
                    ASSERT_NE(results.end(), results.find("172.30.4.118"));
                    ASSERT_EQ("54.85.192.213", results["172.30.4.118"]);
                }
            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL
