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

#include <ostream>
#include <vector>
#include <hazelcast/util/ILogger.h>
#include <hazelcast/client/HazelcastClient.h>
#include <ClientTestSupport.h>

namespace hazelcast {
    namespace client {
        namespace test {
            class LoggerConfigFromFileTest : public ClientTestSupport {
            public:
                LoggerConfigFromFileTest() {
                    // Clear file content before test starts
                    std::ofstream logOutFile("testLog.txt", std::ifstream::trunc);

                    config::LoggerConfig loggerConfig;
                    loggerConfig.setConfigurationFileName("hazelcast/test/resources/logger-config.txt");

                    testLogger.reset(new util::ILogger("testConfigureFromFile", "testConfigureFromFile", "testversion",
                                                   loggerConfig));

                }

            protected:
                virtual void SetUp() {
                    ASSERT_TRUE(testLogger->start());

                    originalStdout = std::cout.rdbuf();
                    std::cout.rdbuf(buffer.rdbuf());
                }

                virtual void TearDown() {
                    std::cout.rdbuf(originalStdout);
                }

                std::vector<std::string> getLogLines() {
                    std::string logFileName("testLog.txt");

                    std::ifstream logFile(logFileName.c_str());
                    std::vector<std::string> lines;
                    std::string line;
                    while (std::getline(logFile, line)) {
                        lines.push_back(line);
                    }

                    return lines;
                }

            protected:
                std::streambuf *originalStdout;
                std::stringstream buffer;
                std::unique_ptr<util::ILogger> testLogger;
            };

            TEST_F(LoggerConfigFromFileTest, testFinest) {
                const std::string log("First finest log");
                testLogger->finest(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testFinest2) {
                const std::string log("First finest log");
                testLogger->info(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testInfo) {
                const std::string log("First info log");
                testLogger->info(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testInfo2) {
                const std::string log("First info log");
                testLogger->info(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testWarning) {
                const std::string log("First warning log");
                testLogger->warning(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(0U, lines.size());
            }

            TEST_F(LoggerConfigFromFileTest, testWarning2) {
                const std::string log("First warning log");
                testLogger->warning(log);

                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(0U, lines.size());
            }

            TEST_F(LoggerConfigFromFileTest, testMultipleLinesLog) {
                const std::string infoLog("First Info log");
                testLogger->info(infoLog);

                const std::string debugLog("First debug log");
                testLogger->finest(debugLog);

                const std::string firstFatalLog("First fatal log");
                testLogger->severe(firstFatalLog);

                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(3U, lines.size());

                ASSERT_NE(std::string::npos, lines[0].find(infoLog));

                ASSERT_NE(std::string::npos, lines[1].find(debugLog));

                ASSERT_NE(std::string::npos, lines[2].find(firstFatalLog));

                testLogger->warning("This log should not be printed");

                lines = getLogLines();
                ASSERT_EQ(3U, lines.size());

                testLogger->warning("This log should not be printed");
                lines = getLogLines();
                ASSERT_EQ(3U, lines.size());

                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());

            }

            TEST_F(LoggerConfigFromFileTest, testNonExistingConfigurationFileFailFast) {
                ClientConfig clientConfig;
                clientConfig.getLoggerConfig().setConfigurationFileName("NonExistent");
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

        }
    }
}
