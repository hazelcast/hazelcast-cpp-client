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

#include <gtest/gtest.h>
#include <hazelcast/util/ILogger.h>
#include <ostream>

namespace hazelcast {
    namespace client {
        namespace test {
            class LoggerTest : public ::testing::Test {
            public:
                LoggerTest() : logger(util::ILogger::getLogger()) {
                }

            protected:
                virtual void SetUp() {
                    originalStdout = std::cout.rdbuf();

                    std::cout.rdbuf(buffer.rdbuf());
                }

                virtual void TearDown() {
                    std::cout.rdbuf(originalStdout);
                    logger.setLogLevel(LoggerLevel::INFO);
                }

                class TestObject {
                public:
                    TestObject(int objectCount) : objectCount(objectCount) {}

                    friend std::ostream &operator<<(std::ostream &os, const TestObject &object) {
                        os << "objectCount: " << object.objectCount;
                        return os;
                    }

                private:
                    int objectCount;
                };

                util::ILogger &logger;
                std::stringstream buffer;
                std::streambuf *originalStdout;
                LoggerLevel::Level originalLogLevel;
            };

            TEST_F(LoggerTest, testPrintObject) {
                logger.info() << "This is an info message. " << TestObject(5);

                const std::string &printedString = buffer.str();
                size_t index = printedString.find("This is an info message. objectCount: 5");

                ASSERT_NE(printedString.npos, index);
            }

            TEST_F(LoggerTest, testDefaultLogLevel) {
                ASSERT_FALSE(logger.isFinestEnabled());
                ASSERT_FALSE(logger.isEnabled(client::LoggerLevel::FINEST));
                ASSERT_TRUE(logger.isEnabled(client::LoggerLevel::INFO));
                ASSERT_TRUE(logger.isEnabled(client::LoggerLevel::WARNING));
                ASSERT_TRUE(logger.isEnabled(client::LoggerLevel::SEVERE));

                logger.finest("Warning level message");
                ASSERT_TRUE(buffer.str().empty());

                logger.info("info message");
                std::string value = buffer.str();
                ASSERT_NE(std::string::npos, value.find("info message"));

                logger.warning("warning message");
                ASSERT_NE(std::string::npos, buffer.str().find("warning message"));

                logger.severe("severe message");
                ASSERT_NE(std::string::npos, buffer.str().find("severe message"));
            }

            TEST_F(LoggerTest, testLogLevel) {
                logger.setLogLevel(client::LoggerLevel::WARNING);

                ASSERT_FALSE(logger.isFinestEnabled());
                ASSERT_FALSE(logger.isEnabled(client::LoggerLevel::FINEST));
                ASSERT_FALSE(logger.isEnabled(client::LoggerLevel::INFO));
                ASSERT_TRUE(logger.isEnabled(client::LoggerLevel::WARNING));
                ASSERT_TRUE(logger.isEnabled(client::LoggerLevel::SEVERE));

                logger.finest("Warning level message");
                ASSERT_TRUE(buffer.str().empty());

                logger.info("info message");
                ASSERT_TRUE(buffer.str().empty());

                logger.warning("warning message");
                ASSERT_NE(std::string::npos, buffer.str().find("warning message"));

                logger.severe("severe message");
                ASSERT_NE(std::string::npos, buffer.str().find("severe message"));
            }
        }
    }
}

