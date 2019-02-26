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
#include <gtest/gtest.h>
#include <hazelcast/client/exception/IOException.h>
#include <hazelcast/client/protocol/ClientExceptionFactory.h>
#include <hazelcast/util/IOUtil.h>

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class ExceptionTest : public ::testing::Test {
                protected:
                };

                TEST_F(ExceptionTest, testExceptionCause) {
                    boost::shared_ptr<exception::IException> cause = boost::shared_ptr<exception::IException>(
                            new exception::IOException("testExceptionCause test", "this is a test exception"));
                    exception::TargetDisconnectedException targetDisconnectedException("testExceptionCause",
                                                                                       "test message", cause);

                    boost::shared_ptr<exception::IException> exceptionCause = targetDisconnectedException.getCause();
                    ASSERT_NE(static_cast<exception::IException *>(NULL), exceptionCause.get());
                    ASSERT_THROW((exceptionCause->raise()), exception::IOException);
                    ASSERT_EQ(exceptionCause->getMessage(), cause->getMessage());
                    ASSERT_EQ(exceptionCause->getSource(), cause->getSource());
                    ASSERT_EQ((exception::IException *) NULL, exceptionCause->getCause().get());
                }

                TEST_F(ExceptionTest, testExceptionDetail) {
                    std::string details("A lot of details");
                    exception::TargetDisconnectedException targetDisconnectedException("testExceptionCause",
                                                                                       "test message", details,
                                                                                       protocol::IO);


                    ASSERT_EQ(details, targetDisconnectedException.getDetails());
                }

                TEST_F(ExceptionTest, testExceptionBuilderBuildShared) {
                    boost::shared_ptr<exception::IOException> sharedException = exception::ExceptionBuilder<exception::IOException>(
                            "Exception from testExceptionBuilderBuildShared").buildShared();
                    ASSERT_THROW(throw *sharedException, exception::IOException);
                }

                TEST_F(ExceptionTest, testExceptionStreaming) {
                    std::string source("testException");
                    std::string originalMessage("original message");
                    exception::IOException e(source, originalMessage);

                    ASSERT_EQ(source, e.getSource());
                    ASSERT_EQ(originalMessage, e.getMessage());
                    ASSERT_EQ(static_cast<exception::IException *>(NULL), e.getCause().get());

                    std::string extendedMessage(" this is an extension message");
                    int messageNumber = 1;
                    exception::IOException ioException = (exception::ExceptionBuilder<exception::IOException>(source)
                            << originalMessage << extendedMessage << messageNumber).build();

                    ASSERT_EQ(source, ioException.getSource());
                    ASSERT_EQ(
                            originalMessage + extendedMessage + hazelcast::util::IOUtil::to_string<int>(messageNumber),
                            ioException.getMessage());
                    ASSERT_EQ(static_cast<exception::IException *>(NULL), e.getCause().get());
                }

                TEST_F(ExceptionTest, testRaiseException) {
                    std::string source("testException");
                    std::auto_ptr<std::string> originalMessage(new std::string("original message"));
                    std::string details("detail message");
                    int32_t code = protocol::IO;
                    int32_t causeCode = protocol::ILLEGAL_STATE;

                    protocol::ExceptionFactoryImpl<exception::IOException> ioExceptionFactory;
                    boost::shared_ptr<exception::IException> exception(ioExceptionFactory.createException(source,
                                                                                                          originalMessage,
                                                                                                          details,
                                                                                                          causeCode));

                    try {
                        exception->raise();
                    } catch (exception::IOException &e) {
                        ASSERT_EQ(source, e.getSource());
                        ASSERT_EQ(*originalMessage + ". Details:" + details, e.getMessage());
                        ASSERT_EQ(code, e.getErrorCode());
                        ASSERT_EQ(causeCode, e.getCauseErrorCode());
                    }
                }
            }
        }
    }
}
