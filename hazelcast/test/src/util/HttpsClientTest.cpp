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

#include <gtest/gtest.h>

#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/client/exception/IOException.h"

namespace hazelcast {
    namespace client {
        namespace test {
            class HttpsClientTest : public ::testing::Test {
            };

            TEST_F(HttpsClientTest, testConnect) {
                util::SyncHttpsClient httpsClient("localhost", "non_existentURL/no_page");
                ASSERT_THROW(httpsClient.openConnection(), client::exception::IOException);
            }

            TEST_F(HttpsClientTest, testConnectToGithub) {
                util::SyncHttpsClient httpsClient("ec2.us-east-1.amazonaws.com", "/?Action=DescribeInstances&Version=2014-06-15&X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIU5IAVNR6X75ARYQ%2F20170413%2Fus-east-1%2Fec2%2Faws4_request&X-Amz-Date=20170413T083821Z&X-Amz-Expires=30&X-Amz-Signature=dff261333170c81ecb21f3a0d5820147233197a32c&X-Amz-SignedHeaders=host");
                try {
                    httpsClient.openConnection();
                } catch (exception::IException &e) {
                    const std::string &msg = e.getMessage();
                    ASSERT_NE(std::string::npos, msg.find("status: 401"));
                }
                ASSERT_THROW(httpsClient.openConnection(), exception::IOException);
            }
        }
    }
}

