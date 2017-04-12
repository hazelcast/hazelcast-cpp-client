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
//
// Created by İhsan Demir on 17/05/15.
//

#include <cmath>
#include <gtest/gtest.h>

#include <hazelcast/client/ClientConfig.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace config {
                class AwsConfigTest : public ::testing::Test {
                };

                TEST_F (AwsConfigTest, testDefaultValues) {
                    client::config::ClientAwsConfig awsConfig;
                    ASSERT_EQ("", awsConfig.getAccessKey());
                    ASSERT_EQ("us-east-1", awsConfig.getRegion());
                    ASSERT_EQ("ec2.amazonaws.com", awsConfig.getHostHeader());
                    ASSERT_EQ("", awsConfig.getIamRole());
                    ASSERT_EQ("", awsConfig.getSecretKey());
                    ASSERT_EQ("", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("", awsConfig.getTagKey());
                    ASSERT_EQ("", awsConfig.getTagValue());
                    ASSERT_EQ(5, awsConfig.getConnectionTimeoutSeconds());
                    ASSERT_FALSE(awsConfig.isInsideAws());
                    ASSERT_FALSE(awsConfig.isEnabled());
                }

                TEST_F (AwsConfigTest, testSetValues) {
                    client::config::ClientAwsConfig awsConfig;

                    awsConfig.setAccessKey("mykey");
                    awsConfig.setRegion("myregion");
                    awsConfig.setHostHeader("myheader");
                    awsConfig.setIamRole("myrole");
                    awsConfig.setSecretKey("mysecret");
                    awsConfig.setSecurityGroupName("mygroup");
                    awsConfig.setTagKey("mytagkey");
                    awsConfig.setTagValue("mytagvalue");
                    awsConfig.setConnectionTimeoutSeconds(3);
                    awsConfig.setInsideAws(true);
                    awsConfig.setEnabled(true);

                    ASSERT_EQ("mykey", awsConfig.getAccessKey());
                    ASSERT_EQ("myregion", awsConfig.getRegion());
                    ASSERT_EQ("myheader", awsConfig.getHostHeader());
                    ASSERT_EQ("myrole", awsConfig.getIamRole());
                    ASSERT_EQ("mysecret", awsConfig.getSecretKey());
                    ASSERT_EQ("mygroup", awsConfig.getSecurityGroupName());
                    ASSERT_EQ("mytagkey", awsConfig.getTagKey());
                    ASSERT_EQ("mytagvalue", awsConfig.getTagValue());
                    ASSERT_EQ(3, awsConfig.getConnectionTimeoutSeconds());
                    ASSERT_TRUE(awsConfig.isInsideAws());
                    ASSERT_TRUE(awsConfig.isEnabled()) << awsConfig;
                }

                TEST_F (AwsConfigTest, testSetEmptyValues) {
                    client::config::ClientAwsConfig awsConfig;

                    ASSERT_THROW(awsConfig.setAccessKey(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setRegion(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setHostHeader(""), exception::IllegalArgumentException);
                    ASSERT_THROW(awsConfig.setSecretKey(""), exception::IllegalArgumentException);
                }

                TEST_F (AwsConfigTest, testClientConfigUsage) {
                    ClientConfig clientConfig;
                    client::config::ClientAwsConfig &awsConfig = clientConfig.getNetworkConfig().getAwsConfig();
                    awsConfig.setEnabled(true);

                    ASSERT_TRUE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());

                    client::config::ClientAwsConfig newConfig;

                    clientConfig.getNetworkConfig().setAwsConfig(newConfig);
                    // default constructor sets enabled to false
                    ASSERT_FALSE(clientConfig.getNetworkConfig().getAwsConfig().isEnabled());
                }
            }
        }
    }
}
