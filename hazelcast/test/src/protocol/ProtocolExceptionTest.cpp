/*
 * Copyright (c) 2008-2018, Hazelcast, Inc. All Rights Reserved.
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
// Created by ihsan Demir on 23 May 2016.
//

#ifndef HAZELCAST_CLIENT_TEST_PROTOCOL_PROTOCOLEXCEPTIONTEST_H
#define HAZELCAST_CLIENT_TEST_PROTOCOL_PROTOCOLEXCEPTIONTEST_H

#include <stdint.h>
#include <gtest/gtest.h>

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/protocol/ClientProtocolErrorCodes.h"

namespace hazelcast {
    namespace client {
        namespace test {
            namespace protocol {
                TEST(ProtocolExceptionTest, testUndefinedErrorCodeException) {
                    int32_t undefinedError = client::protocol::SERVICE_NOT_FOUND + 1;
                    int64_t callId = 0x1122334455667788LL;
                    const std::string details = "This is the detail about the exception";
                    client::exception::UndefinedErrorCodeException exception("testUndefinedErrorCodeException",
                                                                             "this is a test", undefinedError, callId,
                                                                             details);
                    ASSERT_EQ(undefinedError, exception.getErrorCode());
                    ASSERT_EQ(callId, exception.getMessageCallId());
                    ASSERT_EQ(details, exception.getDetailedErrorMessage());
                }
            }
        }
    }
}

#endif //HAZELCAST_CLIENT_TEST_PROTOCOL_PROTOCOLEXCEPTIONTEST_H
