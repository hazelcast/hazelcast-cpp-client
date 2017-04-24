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

                TEST_F (AwsClientTest, testConnectionToAwsCluster) {
                    ClientConfig config;
                    // consume the addresses quickly
                    config.getNetworkConfig().setConnectionTimeout(500);
                    config::ClientAwsConfig &awsConfig = config.getNetworkConfig().getAwsConfig();
                    awsConfig.setEnabled(true).setAccessKey(getenv("HZ_TEST_AWS_ACCESS_KEY")).
                            setSecretKey(getenv("HZ_TEST_AWS_SECRET")).setTagKey("Name").setTagValue("*linux-release");
                    try {
                        HazelcastClient hazelcastClient(config);
                    } catch (exception::IException &e) {
                        ASSERT_STREQ("HazelcastClient could not be started!", e.getMessage().c_str());
                    }
                }
            }
        }
    }
}
#endif // HZ_BUILD_WITH_SSL
