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

                    IMap<int, int> map = hazelcastClient.getMap<int, int>("myMap");
                    map.put(5, 20);
                    boost::shared_ptr<int> val = map.get(5);
                    ASSERT_NE((int *) NULL, val.get());
                    ASSERT_EQ(20, *val);
                }

                TEST_F (AwsClientTest, testClientAwsMemberWithSecurityGroupDefaultIamRole) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).
                            setSecurityGroupName("launch-wizard-147");

                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    // The access key and secret will be retrieved from default IAM role at windows machine
                    clientConfig.getNetworkConfig().getAwsConfig().setInsideAws(true);
                    #else
                    clientConfig.getNetworkConfig().getAwsConfig().setAccessKey(getenv("AWS_ACCESS_KEY_ID")).
                            setSecretKey(getenv("AWS_SECRET_ACCESS_KEY"));
                    #endif

                    HazelcastClient hazelcastClient(clientConfig);

                    IMap<int, int> map = hazelcastClient.getMap<int, int>("myMap");
                    map.put(5, 20);
                    boost::shared_ptr<int> val = map.get(5);
                    ASSERT_NE((int *) NULL, val.get());
                    ASSERT_EQ(20, *val);
                }

                /**
                 * Following test can only run from inside the AWS network
                 */
                #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                TEST_F (AwsClientTest, testRetrieveCredentialsFromIamRoleAndConnect) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setIamRole("cloudbees-role").setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }

                TEST_F (AwsClientTest, testRetrieveCredentialsFromInstanceProfileDefaultIamRoleAndConnect) {
                    ClientConfig clientConfig;

                    clientConfig.getProperties()[ClientProperties::PROP_AWS_MEMBER_PORT] = "60000";
                    clientConfig.getNetworkConfig().getAwsConfig().setEnabled(true).setTagKey(
                            "aws-test-tag").setTagValue("aws-tag-value-1").setInsideAws(true);

                    HazelcastClient hazelcastClient(clientConfig);
                }
                #endif
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL
