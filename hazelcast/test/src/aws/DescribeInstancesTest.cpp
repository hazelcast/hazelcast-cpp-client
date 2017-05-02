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

#include <stdlib.h>
#include <gtest/gtest.h>

#include <hazelcast/client/config/ClientAwsConfig.h>
#include <hazelcast/client/aws/impl/DescribeInstances.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace aws {
                class DescribeInstancesTest : public ::testing::Test {
                };

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesTagAndNonExistentValueSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag").setTagValue("non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("aws-test-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyTagIsSetToNonExistentTag) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagKey("non-existent-tag");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSet) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("aws-tag-value-1");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesOnlyValueIsSetToNonExistentValue) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setTagValue("non-existent-value");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("launch-wizard-147");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_EQ(results.size(), 1U);
                    ASSERT_NE(results.end(), results.find(getenv("HZ_TEST_AWS_INSTANCE_PRIVATE_IP")));
                }

                TEST_F (DescribeInstancesTest, testDescribeInstancesNonExistentSecurityGroup) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true).setAccessKey(getenv("AWS_ACCESS_KEY_ID")).setSecretKey(
                            getenv("AWS_SECRET_ACCESS_KEY")).setSecurityGroupName("non-existent-group");
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_TRUE(results.empty());
                }
            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL
