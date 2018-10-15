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
                    config::LoggerConfig loggerConfig;
                    loggerConfig.setConfigurationFileName("hazelcast/test/resources/logger-config.txt");

                    logger.reset(new util::ILogger("testConfigureFromFile", "testConfigureFromFile", "testversion",
                                                   loggerConfig));

                }

            protected:
                virtual void SetUp() {
                    originalStdout = std::cout.rdbuf();

                    std::cout.rdbuf(buffer.rdbuf());
                }

                virtual void TearDown() {
                    std::cout.rdbuf(originalStdout);
                }

                void forceSyncToFileSystem(const std::string &filename) {
                    FILE *fhandle = fopen(filename.c_str(), "r");
                    #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                    FlushFileBuffers(fhandle);
                    #else
                        fsync(fileno(fhandle));
                    #endif
                    fclose(fhandle);
                }

                std::vector<std::string> getLogLines() {
                    std::string logFileName("testLog.txt");

                    forceSyncToFileSystem(logFileName);

                    std::ifstream logFile(logFileName);
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
                std::auto_ptr<util::ILogger> testLogger;
            };

            TEST_F(LoggerConfigFromFileTest, testFinest) {
                const std::string log("First finest log");
                logger->finest(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testFinest2) {
                const std::string log("First finest log");
                {
                    logger->info() << log;
                }
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testInfo) {
                const std::string log("First info log");
                logger->info(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testInfo2) {
                const std::string log("First info log");
                {
                    logger->info() << log;
                }
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(1U, lines.size());
                ASSERT_NE(std::string::npos, lines[0].find(log));
                // make sure that nothing is printed to stdout
                ASSERT_TRUE(buffer.str().empty());
            }

            TEST_F(LoggerConfigFromFileTest, testWarning) {
                const std::string log("First warning log");
                logger->warning(log);
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(0U, lines.size());
            }

            TEST_F(LoggerConfigFromFileTest, testWarning2) {
                const std::string log("First warning log");
                {
                    logger->warning() << log;
                }
                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(0U, lines.size());
            }

            TEST_F(LoggerConfigFromFileTest, testMultipleLinesLog) {
                const std::string infoLog("First Info log");
                logger->info(infoLog);

                const std::string debugLog("First debug log");
                logger->finest(debugLog);

                const std::string firstFatalLog("First fatal log");
                logger->severe(firstFatalLog);

                std::vector<std::string> lines = getLogLines();
                ASSERT_EQ(3U, lines.size());

                ASSERT_NE(std::string::npos, lines[0].find(infoLog));

                ASSERT_NE(std::string::npos, lines[1].find(debugLog));

                ASSERT_NE(std::string::npos, lines[2].find(firstFatalLog));

                {
                    logger->warning() << "This log should not be printed";
                }

                lines = getLogLines();
                ASSERT_EQ(3U, lines.size());

                logger->warning("This log should not be printed");
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

             TEST_F(LoggerConfigFromFileTest, testInvalidConfigurationFileFailFast) {
                ClientConfig clientConfig;
                clientConfig.getLoggerConfig().setConfigurationFileName("hazelcast/test/resources/invalid-logger-config.txt");
                ASSERT_THROW(HazelcastClient client(clientConfig), exception::IllegalStateException);
            }

        }
    }
}
