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

                /**
                 * This test will be enabled when the credentials are provided from environment
                 */
                TEST_F (DescribeInstancesTest, testDescribeInstances) {
                    client::config::ClientAwsConfig awsConfig;
                    awsConfig.setEnabled(true);

                    awsConfig.setAccessKey(getenv("HZ_TEST_AWS_ACCESS_KEY"));
                    awsConfig.setSecretKey(getenv("HZ_TEST_AWS_SECRET"));
                    client::aws::impl::DescribeInstances desc(awsConfig, awsConfig.getHostHeader());
                    std::map<std::string, std::string> results = desc.execute();
                    ASSERT_GT(results.size(), 0U);
                }
            }
        }
    }
}

#endif //HZ_BUILD_WITH_SSL
