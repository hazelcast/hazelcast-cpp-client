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

#include <cmath>
#include <gtest/gtest.h>

#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/ClientConfig.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class AwsClientTest : public ::testing::Test {
                };

                TEST_F (AwsClientTest, testClientAwsMemberNonDefaultPortConfig) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(getenv("AWS_SECRET_ACCESS_KEY")).
                            setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");

                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
                    #else
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(false);
                    #endif

                    HazelcastClient hazelcastClient(clientConfig);
                }
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL
