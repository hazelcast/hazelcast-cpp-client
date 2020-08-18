/*
 * Copyright (c) 2008-2020, Hazelcast, Inc. All Rights Reserved.
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
#include "HazelcastServerFactory.h"
#include "HazelcastServer.h"
#include "ClientTestSupport.h"
#include <vector>
#include "ClientTestSupportBase.h"
#include <hazelcast/client/ClientConfig.h>
#include <hazelcast/client/HazelcastClient.h>
#include <hazelcast/client/serialization/serialization.h>
#include <hazelcast/util/UuidUtil.h>
#include <hazelcast/client/impl/Partition.h>
#include <gtest/gtest.h>
#include <hazelcast/client/connection/ClientConnectionManagerImpl.h>
#include <hazelcast/client/protocol/Principal.h>
#include <hazelcast/client/connection/Connection.h>
#include <hazelcast/client/serialization/pimpl/DataInput.h>
#include <hazelcast/util/AddressHelper.h>
#include <hazelcast/util/Util.h>
#include <TestHelperFunctions.h>
#include <ostream>
#include <hazelcast/util/ILogger.h>
#include <ctime>
#include <errno.h>
#include <hazelcast/client/LifecycleListener.h>
#include "serialization/Serializables.h"
#include <hazelcast/client/SerializationConfig.h>
#include <hazelcast/client/HazelcastJsonValue.h>
#include <hazelcast/client/internal/nearcache/impl/NearCacheRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheDataRecordStore.h>
#include <hazelcast/client/internal/nearcache/impl/store/NearCacheObjectRecordStore.h>
#include <cmath>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/thread/barrier.hpp>

#ifdef HZ_BUILD_WITH_SSL
#include <openssl/crypto.h>
#endif

#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/client/internal/socket/SSLSocket.h"
#include "hazelcast/client/InitialMembershipEvent.h"
#include "hazelcast/client/MemberAttributeEvent.h"
#include "hazelcast/client/SocketInterceptor.h"
#include "hazelcast/client/Socket.h"
#include "hazelcast/client/IMap.h"
#include "hazelcast/util/Bits.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/util/BlockingConcurrentQueue.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/util/ConcurrentQueue.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/client/Pipelining.h"
#include "hazelcast/util/MurmurHash3.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(disable: 4996) //for unsafe getenv
#endif

namespace hazelcast {
    namespace client {
        namespace test {
            class AddressHelperTest : public ClientTestSupport {
            };

            TEST_F(AddressHelperTest, testGetPossibleSocketAddresses) {
                std::string address("10.2.3.1");
                std::vector<Address> addresses = util::AddressHelper::getSocketAddresses(address, getLogger());
                ASSERT_EQ(3U, addresses.size());
                std::unordered_set<Address> socketAddresses;
                socketAddresses.insert(addresses.begin(), addresses.end());
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(Address(address, 5701)));
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(Address(address, 5702)));
                ASSERT_NE(socketAddresses.end(), socketAddresses.find(Address(address, 5703)));
            }

            TEST_F(AddressHelperTest, testAddressHolder) {
                util::AddressHolder holder("127.0.0.1", "en0", 8000);
                ASSERT_EQ("127.0.0.1", holder.getAddress());
                ASSERT_EQ(8000, holder.getPort());
                ASSERT_EQ("en0", holder.getScopeId());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class ExceptionTest : public ::testing::Test {
                protected:
                };

                TEST_F(ExceptionTest, testExceptionDetail) {
                    std::string details("A lot of details");
                    exception::TargetDisconnectedException targetDisconnectedException("testExceptionCause",
                                                                                       "test message", details);


                    ASSERT_EQ(details, targetDisconnectedException.getDetails());
                }

                TEST_F(ExceptionTest, testExceptionStreaming) {
                    std::string source("testException");
                    std::string originalMessage("original message");
                    exception::IOException e(source, originalMessage);

                    ASSERT_EQ(source, e.getSource());
                    ASSERT_EQ(originalMessage, e.getMessage());

                    std::string extendedMessage(" this is an extension message");
                    int messageNumber = 1;
                    exception::IOException ioException = (exception::ExceptionBuilder<exception::IOException>(source)
                            << originalMessage << extendedMessage << messageNumber).build();

                    ASSERT_EQ(source, ioException.getSource());
                    ASSERT_EQ(
                            originalMessage + extendedMessage + std::to_string(messageNumber),
                            ioException.getMessage());
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class BlockingConcurrentQueueTest : public ::testing::Test
                {
                protected:
                    static void PushDelayed(hazelcast::util::ThreadArgs &args) {
                        hazelcast::util::BlockingConcurrentQueue<int> *q = (hazelcast::util::BlockingConcurrentQueue<int> *)args.arg0;
                        int *val = (int *)args.arg1;
                        unsigned long *milliseconds = (unsigned long *)args.arg2;
                        hazelcast::util::sleepmillis(*milliseconds);
                        q->push(*val);
                    }
                    static void Pop(hazelcast::util::ThreadArgs &args) {
                        hazelcast::util::BlockingConcurrentQueue<int> *q = (hazelcast::util::BlockingConcurrentQueue<int> *)args.arg0;
                        std::atomic<int> *val = (std::atomic<int> *)args.arg1;
                        val->store(q->pop());
                    }

                };

                TEST_F(BlockingConcurrentQueueTest, testPushDelyaed) {
                    size_t capacity = 3;

                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_NO_THROW(q.push(i));
                    }

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_EQ((int) i, q.pop());
                    }

                    int val = 7;

                    unsigned long sleepTime = 3000U;
                    hazelcast::util::StartedThread t(PushDelayed, &q, &val, &sleepTime);

                    int64_t start = hazelcast::util::currentTimeMillis();
                    ASSERT_EQ(val, q.pop());
                    int64_t diff = hazelcast::util::currentTimeMillis() - start;
                    ASSERT_TRUE(diff > 1000);
                }

                TEST_F(BlockingConcurrentQueueTest, testPop) {
                    size_t capacity = 3;

                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_NO_THROW(q.push(i));
                    }

                    for (size_t i = 0; i < capacity; ++i) {
                        ASSERT_EQ((int) i, q.pop());
                    }

                    std::atomic<int> val(-1);
                    int testValue = 7;
                    unsigned long sleepTime = 3000U;
                    hazelcast::util::StartedThread t(Pop, &q, &val, &sleepTime);

                    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
                    ASSERT_NO_THROW(q.push(testValue));
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    ASSERT_EQ(testValue, (int) val);
                }

                TEST_F(BlockingConcurrentQueueTest, testInterrupt) {
                    size_t capacity = 3;
                    hazelcast::util::BlockingConcurrentQueue<int> q(capacity);

                    std::atomic<bool> finished{false};
                    std::thread t([&] () {
                        while (!finished) {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            q.interrupt();
                        }
                    });
                    // Note that this test is time sensitive, this thread shoulc be waiting at blocking pop when the
                    // other thread executes the interrup call.
                    ASSERT_THROW(q.pop(), client::exception::InterruptedException);
                    finished = true;
                    t.join();
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class UuidUtilTest : public ::testing::Test {
            };

            TEST_F(UuidUtilTest, testUnsecureUuid) {
                hazelcast::util::UUID uuid1 = hazelcast::util::UuidUtil::newUnsecureUUID();
                hazelcast::util::UUID uuid2 = hazelcast::util::UuidUtil::newUnsecureUUID();
                ASSERT_NE(uuid1, uuid2);

                std::string uuid1String = uuid1.toString();
                std::string uuid2String = uuid2.toString();
                ASSERT_NE(uuid1String, uuid2String);
                ASSERT_EQ(36U, uuid1String.length());
                ASSERT_EQ(36U, uuid2String.length());

                std::stringstream ss(uuid1String);
                std::string token;
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(8U, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(4U, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(4U, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(4U, token.length());
                ASSERT_TRUE(std::getline(ss, token, '-'));
                ASSERT_EQ(12U, token.length());
                ASSERT_FALSE(std::getline(ss, token, '-'));
            }

            TEST_F(UuidUtilTest, testUuidToString) {
                int64_t msb = static_cast<int64_t>(0xfb34567812345678LL);
                int64_t lsb = static_cast<int64_t>(0xabcd123412345678LL);
                hazelcast::util::UUID uuid(msb, lsb);
                std::string uuidString = uuid.toString();
                ASSERT_EQ("fb345678-1234-5678-abcd-123412345678", uuidString);
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class UTFUtilTest : public ::testing::Test {
                protected:
                    // includes one, two three and 4 byte code points
                    static const std::string VALID_UTF_STRING;

                    static const std::string INVALID_UTF_STRING_INSUFFICIENT_BYTES;
                };

                const std::string UTFUtilTest::VALID_UTF_STRING = "a \xc3\xa9 \xe5\x92\xa7 \xf6\xa7\x93\xb5";
                const std::string UTFUtilTest::INVALID_UTF_STRING_INSUFFICIENT_BYTES = "a \xc3\xa9 \xe5\x92 \xf6\xa7\x93\xb5";

                TEST_F(UTFUtilTest, validUTF8) {
                    ASSERT_GT(hazelcast::util::UTFUtil::isValidUTF8(VALID_UTF_STRING), 0);
                }

                TEST_F(UTFUtilTest, invalidUTF8) {
                    ASSERT_EQ(-1, hazelcast::util::UTFUtil::isValidUTF8(INVALID_UTF_STRING_INSUFFICIENT_BYTES));
                }

                TEST_F(UTFUtilTest, readValidUTF8) {
                    serialization::pimpl::DataInput<std::string> in(VALID_UTF_STRING);
                    std::string utfBuffer;
                    int numberOfUtfChars = hazelcast::util::UTFUtil::isValidUTF8(VALID_UTF_STRING);
                    for (int i = 0; i < numberOfUtfChars; ++i) {
                        byte c = in.read<byte>();
                        hazelcast::util::UTFUtil::readUTF8Char(in, c, utfBuffer);
                    }

                    std::string result(utfBuffer.begin(), utfBuffer.end());
                    ASSERT_EQ(VALID_UTF_STRING, result);
                }

                TEST_F(UTFUtilTest, readInvalidUTF8) {
                    serialization::pimpl::DataInput<std::string> in(INVALID_UTF_STRING_INSUFFICIENT_BYTES);
                    std::string utfBuffer;
                    for (int i = 0; i < 5; ++i) {
                        byte c = in.read<byte>();
                        // The 4th utf character is missing one byte intentionally in the invalid utf string
                        if (i == 4) {
                            ASSERT_THROW(hazelcast::util::UTFUtil::readUTF8Char(in, c, utfBuffer),
                                         exception::UTFDataFormatException);
                        } else {
                            hazelcast::util::UTFUtil::readUTF8Char(in, c, utfBuffer);
                        }
                    }
                }
            }
        }
    }
}

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

                    testLogger.reset(new hazelcast::util::ILogger("testConfigureFromFile", "testConfigureFromFile",
                                                                  "testversion",
                                                                  loggerConfig));

                }

            protected:
                void SetUp() override {
                    ASSERT_TRUE(testLogger->start());

                    originalStdout = std::cout.rdbuf();
                    std::cout.rdbuf(buffer.rdbuf());
                }

                void TearDown() override {
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
                std::unique_ptr<hazelcast::util::ILogger> testLogger;
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

namespace hazelcast {
    namespace client {
        namespace test {
            class LoggerTest : public ClientTestSupport {
            public:
                LoggerTest() : logger(getLogger()) {
                }

            protected:
                void SetUp() override {
                    originalStdout = std::cout.rdbuf();

                    std::cout.rdbuf(buffer.rdbuf());
                }

                void TearDown() override {
                    std::cout.rdbuf(originalStdout);
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

                hazelcast::util::ILogger &logger;
                std::stringstream buffer;
                std::streambuf *originalStdout;
            };

            TEST_F(LoggerTest, testPrintObject) {
                logger.info("This is an info message. ", TestObject(5));

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
                const char *testName = testing::UnitTest::GetInstance()->current_test_info()->name();
                config::LoggerConfig loggerConfig;
                loggerConfig.setLogLevel(client::LoggerLevel::WARNING);
                std::shared_ptr<hazelcast::util::ILogger> logger(
                        new hazelcast::util::ILogger(testName, testName, "TestVersion", loggerConfig));
                ASSERT_TRUE(logger->start());

                ASSERT_FALSE(logger->isFinestEnabled());
                ASSERT_FALSE(logger->isEnabled(client::LoggerLevel::FINEST));
                ASSERT_FALSE(logger->isEnabled(client::LoggerLevel::INFO));
                ASSERT_TRUE(logger->isEnabled(client::LoggerLevel::WARNING));
                ASSERT_TRUE(logger->isEnabled(client::LoggerLevel::SEVERE));

                logger->finest("Warning level message");
                ASSERT_TRUE(buffer.str().empty());

                logger->info("info message");
                ASSERT_TRUE(buffer.str().empty());

                logger->warning("warning message");
                ASSERT_NE(std::string::npos, buffer.str().find("warning message"));

                logger->severe("severe message");
                ASSERT_NE(std::string::npos, buffer.str().find("severe message"));
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace util {
                class ConcurentQueueTest : public ClientTestSupport
                {
                protected:
                    class ConcurrentQueueTask {
                    public:
                        ConcurrentQueueTask(hazelcast::util::ConcurrentQueue<int> &q,
                                            boost::latch &startLatch,
                                            boost::latch &startRemoveLatch, int removalValue) : q(q),
                                                                                                startLatch(
                                                                                                        startLatch),
                                                                                                startRemoveLatch(
                                                                                                        startRemoveLatch),
                                                                                                removalValue(
                                                                                                        removalValue) {}

                        virtual void run() {
                            int numItems = 1000;

                            std::vector<int> values((size_t) numItems);

                            startLatch.count_down();

                            ASSERT_EQ(boost::cv_status::no_timeout, startLatch.wait_for(boost::chrono::seconds(10)));

                            // insert items
                            for (int i = 0; i < numItems; ++i) {
                                values[i] = i;
                                q.offer(&values[i]);
                            }

                            q.offer(&removalValue);
                            startRemoveLatch.count_down();

                            // poll items
                            for (int i = 0; i < numItems; ++i) {
                                values[i] = i;
                                ASSERT_NE((int *) nullptr, q.poll());
                            }
                        }

                        virtual const std::string getName() const {
                            return "ConcurrentQueueTask";
                        }

                    private:
                        hazelcast::util::ConcurrentQueue<int> &q;
                        boost::latch &startLatch;
                        boost::latch &startRemoveLatch;
                        int removalValue;
                    };
                };

                TEST_F(ConcurentQueueTest, testSingleThread) {
                    hazelcast::util::ConcurrentQueue<int> q;

                    ASSERT_EQ((int *) nullptr, q.poll());

                    int val1, val2;

                    q.offer(&val1);

                    ASSERT_EQ(&val1, q.poll());

                    ASSERT_EQ((int *) nullptr, q.poll());

                    q.offer(&val1);
                    q.offer(&val2);
                    q.offer(&val2);
                    q.offer(&val1);

                    ASSERT_EQ(2, q.removeAll(&val2));
                    ASSERT_EQ(0, q.removeAll(&val2));

                    ASSERT_EQ(&val1, q.poll());
                    ASSERT_EQ(&val1, q.poll());

                    ASSERT_EQ((int *) nullptr, q.poll());
                }

                TEST_F(ConcurentQueueTest, testMultiThread) {
                    constexpr int numThreads = 40;

                    boost::latch startLatch(numThreads);

                    boost::latch startRemoveLatch(numThreads);

                    hazelcast::util::ConcurrentQueue<int> q;

                    int removalValue = 10;

                    std::array<std::future<void>, numThreads> allFutures;
                    for (int i = 0; i < numThreads; i++) {
                        allFutures[i] = std::async([&]() {
                            ConcurrentQueueTask(q, startLatch, startRemoveLatch, removalValue).run();
                        });
                    }

                    // wait for the remove start
                    ASSERT_EQ(boost::cv_status::no_timeout, startRemoveLatch.wait_for(boost::chrono::seconds(30)));

                    int numRemoved = q.removeAll(&removalValue);

                    int numRemaining = numThreads - numRemoved;

                    for (int j = 0; j < numRemaining; ++j) {
                        ASSERT_NE((int *) nullptr, q.poll());
                    }
                    ASSERT_EQ(0, q.removeAll(&removalValue));

                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientUtilTest : public ClientTestSupport {
            protected:
            };

            TEST_F (ClientUtilTest, testStrError) {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                                                                                                                                        int error = WSAEINPROGRESS;
                std::string expectedErrorString("A blocking operation is currently executing.\r\n");
#else
                int error = EINPROGRESS;
                std::string expectedErrorString("Operation now in progress");
#endif

                char msg[100];
                const std::string prefix = "testStrError prefix message";
                ASSERT_EQ(0, hazelcast::util::strerror_s(error, msg, 100, prefix.c_str()));
                ASSERT_STREQ((prefix + " " + expectedErrorString).c_str(), msg);

                ASSERT_EQ(0, hazelcast::util::strerror_s(error, msg, prefix.length() + 1, prefix.c_str()));
                ASSERT_STREQ(prefix.c_str(), msg);

                ASSERT_EQ(0, hazelcast::util::strerror_s(error, msg, 100));
                ASSERT_STREQ(expectedErrorString.c_str(), msg);
            }

            TEST_F (ClientUtilTest, testAvailableCoreCount) {
                ASSERT_GT(hazelcast::util::getAvailableCoreCount(), 0);
            }

            TEST_F (ClientUtilTest, testStringUtilTimeToString) {
                std::string timeString = hazelcast::util::StringUtil::timeToString(
                        std::chrono::steady_clock::now());
//expected format is "%Y-%m-%d %H:%M:%S.%f" it will be something like 2018-03-20 15:36:07.280
                ASSERT_EQ((size_t) 23, timeString.length());
                ASSERT_EQ(timeString[0], '2');
                ASSERT_EQ(timeString[1], '0');
                ASSERT_EQ(timeString[4], '-');
                ASSERT_EQ(timeString[7], '-');
            }

            TEST_F (ClientUtilTest, testStringUtilTimeToStringFriendly) {
                ASSERT_EQ("never", hazelcast::util::StringUtil::timeToString(std::chrono::steady_clock::time_point()));
            }

            TEST_F (ClientUtilTest, testLockSupport) {
                int64_t parkDurationNanos = 100;
                int64_t start = hazelcast::util::currentTimeNanos();
                hazelcast::util::concurrent::locks::LockSupport::parkNanos(parkDurationNanos);
                int64_t end = hazelcast::util::currentTimeNanos();
                int64_t actualDuration = end - start;
                ASSERT_GE(actualDuration, parkDurationNanos);
            }
        }
    }
}

namespace hazelcast {
    namespace client {

        class HazelcastClient;

        namespace test {
            class ClientConfigTest : public ClientTestSupport
            {};

            TEST_F(ClientConfigTest, testGetAddresses) {
                ClientConfig clientConfig;
                Address address("localhost", 5555);
                clientConfig.getNetworkConfig().addAddress(address);

                auto addresses = clientConfig.getAddresses();
                ASSERT_EQ(1U, addresses.size());
                ASSERT_EQ(address, *addresses.begin());
            }

            TEST_F(ClientConfigTest, testAddresseses) {
                ClientConfig clientConfig;
                std::vector<Address> addresses{Address("localhost", 5555), Address("localhost", 6666)};
                std::sort(addresses.begin(), addresses.end());
                clientConfig.getNetworkConfig().addAddresses(addresses);

                std::unordered_set<Address> configuredAddresses = clientConfig.getAddresses();
                ASSERT_EQ(2U, addresses.size());
                std::vector<Address> configuredAddressVector(configuredAddresses.begin(), configuredAddresses.end());
                std::sort(configuredAddressVector.begin(), configuredAddressVector.end());
                ASSERT_EQ(addresses, configuredAddressVector);
            }

            TEST_F(ClientConfigTest, testSetGetGroupConfig) {
                ClientConfig clientConfig;
                std::string groupName("myGroup");
                std::string password("myPass");
                GroupConfig groupConfig(groupName, password);
                clientConfig.setGroupConfig(groupConfig);
                GroupConfig &clientGroupConfig = clientConfig.getGroupConfig();
                ASSERT_EQ(groupName, clientGroupConfig.getName());
                ASSERT_EQ(password, clientGroupConfig.getPassword());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace connectionstrategy {

                class ConfiguredBehaviourTest : public ClientTestSupport {
                public:
                    ConfiguredBehaviourTest() {
                        clientConfig.getNetworkConfig().setConnectionTimeout(2000).setConnectionAttemptLimit(2).
                                setConnectionAttemptPeriod(1000);
                    }

                protected:
                    class LifecycleStateListener : public LifecycleListener {
                    public:
                        LifecycleStateListener(boost::latch &connectedLatch,
                                               const LifecycleEvent::LifeCycleState expectedState)
                                : connectedLatch(connectedLatch), expectedState(expectedState) {}

                        void stateChanged(const LifecycleEvent &event) override {
                            if (event.getState() == expectedState) {
                                connectedLatch.try_count_down();
                            }
                        }

                    private:
                        boost::latch &connectedLatch;
                        const LifecycleEvent::LifeCycleState expectedState;
                    };

                    ClientConfig clientConfig;
                };

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrueNoCluster) {
                    clientConfig.getConnectionStrategyConfig().setAsyncStart(true);
                    HazelcastClient client(clientConfig);

                    ASSERT_THROW((client.getMap(randomMapName())),
                                 exception::HazelcastClientOfflineException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrueNoCluster_thenShutdown) {
                    clientConfig.getConnectionStrategyConfig().setAsyncStart(true);
                    HazelcastClient client(clientConfig);
                    client.shutdown();
                    ASSERT_THROW((client.getMap(randomMapName())),
                                 exception::HazelcastClientOfflineException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testAsyncStartTrue) {
                    boost::latch connectedLatch(1);

                    // trying 8.8.8.8 address will delay the initial connection since no such server exist
                    clientConfig.getNetworkConfig().addAddress(Address("8.8.8.8", 5701))
                            .addAddress(Address("127.0.0.1", 5701)).setConnectionAttemptLimit(INT32_MAX);
                    clientConfig.setProperty("hazelcast.client.shuffle.member.list", "false");
                    LifecycleStateListener lifecycleListener(connectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&lifecycleListener);
                    clientConfig.getConnectionStrategyConfig().setAsyncStart(true);

                    HazelcastClient client(clientConfig);

                    ASSERT_TRUE(client.getLifecycleService().isRunning());

                    HazelcastServer server(*g_srvFactory);

                    ASSERT_OPEN_EVENTUALLY(connectedLatch);

                    auto map = client.getMap(randomMapName());
                    map->size().get();

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeOFFSingleMember) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::OFF);
                    HazelcastClient client(clientConfig);
                    boost::latch shutdownLatch(1);
                    LifecycleStateListener lifecycleListener(shutdownLatch, LifecycleEvent::SHUTDOWN);
                    client.addLifecycleListener(&lifecycleListener);

                    // no exception at this point
                    auto map = client.getMap(randomMapName());
                    map->put(1, 5).get();

                    hazelcastInstance.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map->put(1, 5).get(), exception::HazelcastClientNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeOFFTwoMembers) {
                    HazelcastServer ownerServer(*g_srvFactory);

                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::OFF);
                    HazelcastClient client(clientConfig);
                    HazelcastServer hazelcastInstance2(*g_srvFactory);
                    boost::latch shutdownLatch(1);
                    LifecycleStateListener lifecycleListener(shutdownLatch, LifecycleEvent::SHUTDOWN);
                    client.addLifecycleListener(&lifecycleListener);

// no exception at this point
                    auto map = client.getMap(randomMapName());
                    map->put(1, 5).get();

                    ownerServer.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map->put(1, 5).get(), exception::HazelcastClientNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMemberInitiallyOffline) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::OFF);
                    HazelcastClient client(clientConfig);
                    boost::latch shutdownLatch(1);
                    LifecycleStateListener lifecycleListener(shutdownLatch, LifecycleEvent::SHUTDOWN);
                    client.addLifecycleListener(&lifecycleListener);

// no exception at this point
                    auto map = client.getMap(randomMapName());
                    map->put(1, 5).get();

                    hazelcastInstance.shutdown();
                    ASSERT_OPEN_EVENTUALLY(shutdownLatch);

                    ASSERT_THROW(map->put(1, 5).get(), exception::HazelcastClientNotActiveException);

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMember) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    boost::latch connectedLatch(1);

                    LifecycleStateListener listener(connectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&listener);
                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::ASYNC);
                    HazelcastClient client(clientConfig);
                    ASSERT_TRUE(client.getLifecycleService().isRunning());
                    ASSERT_OPEN_EVENTUALLY(connectedLatch);

                    auto map = client.getMap(randomMapName());
                    map->size().get();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCSingleMemberStartLate) {
                    HazelcastServer hazelcastInstance(*g_srvFactory);

                    boost::latch initialConnectionLatch(1);
                    boost::latch reconnectedLatch(1);

                    clientConfig.getNetworkConfig().setConnectionAttemptLimit(INT32_MAX);
                    LifecycleStateListener listener(initialConnectionLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&listener);
                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::ASYNC);
                    HazelcastClient client(clientConfig);

                    ASSERT_OPEN_EVENTUALLY(initialConnectionLatch);

                    hazelcastInstance.shutdown();

                    LifecycleStateListener reconnectListener(reconnectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    client.addLifecycleListener(&reconnectListener);

                    HazelcastServer hazelcastInstance2(*g_srvFactory);

                    ASSERT_TRUE(client.getLifecycleService().isRunning());
                    ASSERT_OPEN_EVENTUALLY(reconnectedLatch);

                    auto map = client.getMap(randomMapName());
                    map->size().get();

                    client.shutdown();
                }

                TEST_F(ConfiguredBehaviourTest, testReconnectModeASYNCTwoMembers) {
                    HazelcastServer ownerServer(*g_srvFactory);

                    boost::latch connectedLatch(1);
                    boost::latch disconnectedLatch(1);
                    boost::latch reconnectedLatch(1);

                    clientConfig.getNetworkConfig().setConnectionAttemptLimit(10);
                    LifecycleStateListener listener(connectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    clientConfig.addListener(&listener);
                    clientConfig.getConnectionStrategyConfig().setReconnectMode(
                            config::ClientConnectionStrategyConfig::ASYNC);
                    HazelcastClient client(clientConfig);

                    ASSERT_TRUE(client.getLifecycleService().isRunning());

                    ASSERT_OPEN_EVENTUALLY(connectedLatch);

                    HazelcastServer hazelcastInstance2(*g_srvFactory);

                    auto map = client.getMap(randomMapName());
                    map->put(1, 5).get();

                    LifecycleStateListener disconnectListener(disconnectedLatch, LifecycleEvent::CLIENT_DISCONNECTED);
                    client.addLifecycleListener(&disconnectListener);

                    LifecycleStateListener reconnectListener(reconnectedLatch, LifecycleEvent::CLIENT_CONNECTED);
                    client.addLifecycleListener(&reconnectListener);

                    ownerServer.shutdown();

                    ASSERT_OPEN_EVENTUALLY(disconnectedLatch);
                    ASSERT_OPEN_EVENTUALLY(reconnectedLatch);

                    map->get<int, int>(1).get();

                    client.shutdown();
                }
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PipeliningTest : public ClientTestSupport {
            public:
                static void SetUpTestCase() {
                    instance = new HazelcastServer(*g_srvFactory);
                    client = new HazelcastClient(ClientConfig());

                    map = client->getMap(MAP_NAME);
                    expected = new std::vector<int>;
                    for (int k = 0; k < MAP_SIZE; ++k) {
                        int item = rand();
                        expected->emplace_back(item);
                        map->put(k, item).get();
                    }
                }

                static void TearDownTestCase() {
                    delete instance;
                    instance = nullptr;
                    delete client;
                    client = nullptr;
                    delete expected;
                    expected = nullptr;
                }

            protected:
                static void testPipelining(const std::shared_ptr<Pipelining<int> > &pipelining) {
                    for (int k = 0; k < MAP_SIZE; k++) {
                        pipelining->add(map->get<int, int>(k));
                    }

                    auto results = pipelining->results();
                    ASSERT_EQ(expected->size(), results.size());
                    for (int k = 0; k < MAP_SIZE; ++k) {
                        ASSERT_TRUE(results[k].has_value());
                        ASSERT_EQ((*expected)[k], results[k].value());
                    }
                }

                static HazelcastServer *instance;
                static HazelcastClient *client;
                static const char *MAP_NAME;
                static std::shared_ptr<IMap> map;
                static std::vector<int> *expected;
                static const int MAP_SIZE = 10000;
            };

            HazelcastServer *PipeliningTest::instance = nullptr;
            std::shared_ptr<IMap> PipeliningTest::map;
            HazelcastClient *PipeliningTest::client = nullptr;
            const char *PipeliningTest::MAP_NAME = "PipeliningTestMap";
            std::vector<int> *PipeliningTest::expected = nullptr;

            TEST_F(PipeliningTest, testConstructor_whenNegativeDepth) {
                ASSERT_THROW(Pipelining<std::string>::create(0), exception::IllegalArgumentException);
                ASSERT_THROW(Pipelining<std::string>::create(-1), exception::IllegalArgumentException);
            }

            TEST_F(PipeliningTest, testPipeliningFunctionalityDepthOne) {
                testPipelining(Pipelining<int>::create(1));
            }

            TEST_F(PipeliningTest, testPipeliningFunctionalityDepth100) {
                testPipelining(Pipelining<int>::create(100));
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PortableVersionTest : public ::testing::Test {
            public:
                class Child {
                public:
                    Child() = default;

                    Child(std::string name) : name(name) {}

                    const std::string &getName() const {
                        return name;
                    }

                    friend bool operator==(const Child &lhs, const Child &rhs) {
                        return lhs.name == rhs.name;
                    }

                private:
                    std::string name;
                };

                class Parent {
                public:
                    friend bool operator==(const Parent &lhs, const Parent &rhs) {
                        return lhs.child == rhs.child;
                    }

                    Parent() = default;

                    Parent(Child child) : child(child) {}

                    const Child &getChild() const {
                        return child;
                    }

                private:
                    Child child;
                };
            };

            // Test for issue https://github.com/hazelcast/hazelcast/issues/12733
            TEST_F(PortableVersionTest, test_nestedPortable_versionedSerializer) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss1(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig2.setPortableVersion(6);
                serialization::pimpl::SerializationService ss2(serializationConfig2);

                //make sure ss2 cached class definition of Child
                ss2.toData<Child>(new Child("sancar"));

                //serialized parent from ss1
                Parent parent(Child("sancar"));
                serialization::pimpl::Data data = ss1.toData<Parent>(&parent);

                // cached class definition of Child and the class definition from data coming from ss1 should be compatible
                ASSERT_EQ(parent, *ss2.toObject<Parent>(data));
            }
        }
        namespace serialization {
            template<>
            struct hz_serializer<test::PortableVersionTest::Child> : public portable_serializer {
                static int32_t getFactoryId() {
                    return 1;
                }

                static int getClassId() {
                    return 2;
                }

                static void writePortable(const test::PortableVersionTest::Child &object,
                                          PortableWriter &writer) {
                    writer.write("name", object.getName());
                }

                static test::PortableVersionTest::Child readPortable(PortableReader &reader) {
                    return test::PortableVersionTest::Child(reader.read<std::string>("name"));
                }
            };

            template<>
            struct hz_serializer<test::PortableVersionTest::Parent> : public portable_serializer {
                static int32_t getFactoryId() {
                    return 1;
                }

                static int32_t getClassId() {
                    return 1;
                }

                static void writePortable(const test::PortableVersionTest::Parent &object, PortableWriter &writer) {
                    writer.writePortable<test::PortableVersionTest::Child>("child", &object.getChild());
                }

                static test::PortableVersionTest::Parent readPortable(PortableReader &reader) {
                    return test::PortableVersionTest::Parent(
                            reader.readPortable<test::PortableVersionTest::Child>("child").value());
                }
            };

        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class PartitionAwareTest : public ClientTestSupport {
            public:
                class SimplePartitionAwareObject : public PartitionAware<int> {
                public:
                    SimplePartitionAwareObject() : testKey(5) {}

                    const int *getPartitionKey() const override {
                        return &testKey;
                    }

                    int getTestKey() const {
                        return testKey;
                    }
                private:
                    int testKey;
                };
            };

            TEST_F(PartitionAwareTest, testSimplePartitionAwareObjectSerialisation) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SimplePartitionAwareObject obj;
                serialization::pimpl::Data data = serializationService.toData<SimplePartitionAwareObject>(&obj);
                ASSERT_TRUE(data.hasPartitionHash());

                int testKey = obj.getTestKey();
                serialization::pimpl::Data expectedData = serializationService.toData<int>(&testKey);

                ASSERT_EQ(expectedData.getPartitionHash(), data.getPartitionHash());
            }

            TEST_F(PartitionAwareTest, testNonPartitionAwareObjectSerialisation) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                int obj = 7;
                serialization::pimpl::Data data = serializationService.toData<int>(&obj);
                ASSERT_FALSE(data.hasPartitionHash());
            }
        }

        namespace serialization {
            template<>
            struct hz_serializer<test::PartitionAwareTest::SimplePartitionAwareObject> : public identified_data_serializer {
                static int32_t getFactoryId() {
                    return 1;
                }

                static int32_t getClassId() {
                    return 2;
                }

                static void writeData(const test::PartitionAwareTest::SimplePartitionAwareObject &object, ObjectDataOutput &out) {}

                static test::PartitionAwareTest::SimplePartitionAwareObject readData(ObjectDataInput &in) {
                    return test::PartitionAwareTest::SimplePartitionAwareObject();
                }
            };
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class JsonValueSerializationTest : public ClientTestSupport {
            public:
                JsonValueSerializationTest() : serializationService(config) {}

            protected:
                serialization::pimpl::SerializationService serializationService;
                SerializationConfig config;
            };

            TEST_F(JsonValueSerializationTest, testSerializeDeserializeJsonValue) {
                HazelcastJsonValue jsonValue("{ \"key\": \"value\" }");
                serialization::pimpl::Data jsonData = serializationService.toData<HazelcastJsonValue>(&jsonValue);
                auto jsonDeserialized = serializationService.toObject<HazelcastJsonValue>(jsonData);
                ASSERT_TRUE(jsonDeserialized.has_value());
                ASSERT_EQ(jsonValue, jsonDeserialized.value());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            class ClientSerializationTest : public ::testing::Test {
            protected:
                TestInnerPortable createInnerPortable() {
                    std::vector<byte> bb = {0, 1, 2};
                    std::vector<char> cc = {'c', 'h', 'a', 'r'};
                    std::vector<bool> ba = {false, true, true, false};
                    std::vector<int16_t> ss = {3, 4, 5};
                    std::vector<int32_t> ii = {9, 8, 7, 6};
                    std::vector<int64_t> ll = {0, 1, 5, 7, 9, 11};
                    std::vector<float> ff = {0.6543f, -3.56f, 45.67f};
                    std::vector<double> dd = {456.456, 789.789, 321.321};
                    TestNamedPortable portableArray[5];
                    std::vector<std::string> stringVector{"イロハニホヘト", "チリヌルヲ", "ワカヨタレソ"};
                    std::vector<TestNamedPortable> nn;
                    for (int i = 0; i < 5; i++) {
                        nn.emplace_back(TestNamedPortable{"named-portable-" + std::to_string(i), i});
                    }
                    return TestInnerPortable{bb, ba, cc, ss, ii, ll, ff, dd, stringVector, nn};
                }

                struct NonSerializableObject {
                    std::string s;

                    friend bool operator==(const NonSerializableObject &lhs, const NonSerializableObject &rhs) {
                        return lhs.s == rhs.s;
                    }
                };

                class DummyGlobalSerializer : public serialization::global_serializer {
                public:
                    void write(const boost::any &object, serialization::ObjectDataOutput &out) override {
                        auto const &obj = boost::any_cast<NonSerializableObject>(object);
                        out.write<std::string>(obj.s);
                    }

                    boost::any read(serialization::ObjectDataInput &in) override {
                        return boost::any(NonSerializableObject{in.read<std::string>()});
                    }
                };

                template<typename T>
                T toDataAndBackToObject(serialization::pimpl::SerializationService &ss, T &value) {
                    serialization::pimpl::Data data = ss.toData<T>(value);
                    return *(ss.toObject<T>(data));
                }

                static const unsigned int LARGE_ARRAY_SIZE;
            };

            const unsigned int ClientSerializationTest::LARGE_ARRAY_SIZE =
                    1 * 1024 * 1024;   // 1 MB. Previously it was 10 MB but then the
            // test fails when using Windows 32-bit DLL
            // library with std::bad_alloc with 10 MB

            TEST_F(ClientSerializationTest, testCustomSerialization) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestCustomXSerializable a{131321};
                serialization::pimpl::Data data = serializationService.toData<TestCustomXSerializable>(&a);
                auto a2 = serializationService.toObject<TestCustomXSerializable>(data);
                ASSERT_TRUE(a2);
                ASSERT_EQ(a, a2.value());

                TestCustomPerson b{"TestCustomPerson"};
                serialization::pimpl::Data data1 = serializationService.toData<TestCustomPerson>(&b);
                auto b2 = serializationService.toObject<TestCustomPerson>(data1);
                ASSERT_TRUE(b2);
                ASSERT_EQ(b, *b2);
            }


            TEST_F(ClientSerializationTest, testRawData) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestDataSerializable ds{123, 's'};
                TestNamedPortable np{"named portable", 34567};
                TestRawDataPortable p{123213, chars, np, 22, "Testing raw portable", ds};

                serialization::pimpl::Data data = serializationService.toData<TestRawDataPortable>(&p);
                auto x = serializationService.toObject<TestRawDataPortable>(data);
                ASSERT_TRUE(x);
                ASSERT_EQ(p, *x);
            }

            TEST_F(ClientSerializationTest, testIdentifiedDataSerializable) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;
                TestDataSerializable np{4, 'k'};
                data = serializationService.toData<TestDataSerializable>(&np);

                auto tnp1 = serializationService.toObject<TestDataSerializable>(data);
                ASSERT_TRUE(tnp1);
                ASSERT_EQ(np, *tnp1);
                int x = 4;
                data = serializationService.toData<int>(&x);
                auto ptr = serializationService.toObject<int>(data);
                ASSERT_TRUE(*ptr);
                int y = *ptr;
                ASSERT_EQ(x, y);
            }

            TEST_F(ClientSerializationTest, testRawDataWithoutRegistering) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                char charA[] = "test chars";
                std::vector<char> chars(charA, charA + 10);
                std::vector<byte> bytes;
                bytes.resize(5, 0);
                TestNamedPortable np{"named portable", 34567};
                TestDataSerializable ds{123, 's'};
                TestRawDataPortable p{123213, chars, np, 22, "Testing raw portable", ds};

                serialization::pimpl::Data data = serializationService.toData<TestRawDataPortable>(&p);
                auto x = serializationService.toObject<TestRawDataPortable>(data);
                ASSERT_TRUE(x);
                ASSERT_EQ(p, *x);
            }

            TEST_F(ClientSerializationTest, testInvalidWrite) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidWritePortable p{2131, 123, "q4edfd"};
                ASSERT_THROW(serializationService.toData<TestInvalidWritePortable>(&p),
                             exception::HazelcastSerializationException);
            }

            TEST_F(ClientSerializationTest, testInvalidRead) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                TestInvalidReadPortable p{2131, 123, "q4edfd"};
                serialization::pimpl::Data data = serializationService.toData<TestInvalidReadPortable>(&p);
                ASSERT_THROW(serializationService.toObject<TestInvalidReadPortable>(data),
                             exception::HazelcastSerializationException);
            }

            TEST_F(ClientSerializationTest, testDifferentVersions) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig2.setPortableVersion(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);

                serialization::pimpl::Data data = serializationService.toData<TestNamedPortable>(
                        TestNamedPortable{"portable-v1", 111});

                serialization::pimpl::Data data2 = serializationService2.toData<TestNamedPortableV2>(
                        TestNamedPortableV2{"portable-v2", 123});

                auto t2 = serializationService2.toObject<TestNamedPortableV2>(data);
                ASSERT_TRUE(t2);
                ASSERT_EQ(std::string("portable-v1"), t2->name);
                ASSERT_EQ(111, t2->k);
                ASSERT_EQ(0, t2->v);

                auto t1 = serializationService.toObject<TestNamedPortable>(data2);
                ASSERT_TRUE(t1);
                ASSERT_EQ(std::string("portable-v2"), t1->name);
                ASSERT_EQ(123, t1->k);

            }

            TEST_F(ClientSerializationTest, testBasicFunctionality) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;

                int x = 3;
                data = serializationService.toData<int>(&x);

                auto returnedInt = serializationService.toObject<int>(data);
                ASSERT_EQ(x, *returnedInt);

                int16_t f = 2;
                data = serializationService.toData<int16_t>(&f);

                auto temp = serializationService.toObject<int16_t>(data);
                ASSERT_EQ(f, *temp);

                TestNamedPortable np{"name", 5};
                data = serializationService.toData<TestNamedPortable>(&np);
                auto tnp1 = serializationService.toObject<TestNamedPortable>(data);
                auto tnp2 = serializationService.toObject<TestNamedPortable>(data);
                ASSERT_TRUE(tnp1);
                ASSERT_EQ(np, *tnp1);
                ASSERT_TRUE(tnp2);
                ASSERT_EQ(np, *tnp2);
                TestInnerPortable inner = createInnerPortable();
                data = serializationService.toData<TestInnerPortable>(inner);
                auto tip1 = serializationService.toObject<TestInnerPortable>(data);
                auto tip2 = serializationService.toObject<TestInnerPortable>(data);
                ASSERT_TRUE(tip1);
                ASSERT_EQ(inner, *tip1);
                ASSERT_TRUE(tip2);
                ASSERT_EQ(inner, *tip2);

                TestMainPortable main{(byte) 113, false, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909,
                                      "this is main portable object created for testing!", inner};
                data = serializationService.toData<TestMainPortable>(&main);

                auto tmp1 = serializationService.toObject<TestMainPortable>(data);
                auto tmp2 = serializationService.toObject<TestMainPortable>(data);
                ASSERT_TRUE(tmp1);
                ASSERT_TRUE(tmp2);
                ASSERT_EQ(main, *tmp1);
                ASSERT_EQ(main, *tmp2);
            }

            TEST_F(ClientSerializationTest, testStringLiterals) {
                auto literal = R"delimeter(My example string literal)delimeter";
                serialization::pimpl::SerializationService serializationService(SerializationConfig{});
                auto data = serializationService.toData(literal);
                auto obj = serializationService.toObject<decltype(literal)>(data);
                ASSERT_TRUE(obj);
                ASSERT_EQ(obj.value(), literal);
            }

            TEST_F(ClientSerializationTest, testBasicFunctionalityWithLargeData) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                serialization::pimpl::Data data;

                std::vector<byte> bb(LARGE_ARRAY_SIZE);
                std::vector<bool> ba(LARGE_ARRAY_SIZE);
                std::vector<char> cc(LARGE_ARRAY_SIZE);
                std::vector<int16_t> ss(LARGE_ARRAY_SIZE);
                std::vector<int32_t> ii(LARGE_ARRAY_SIZE);
                std::vector<int64_t> ll(LARGE_ARRAY_SIZE);
                std::vector<float> ff(LARGE_ARRAY_SIZE);
                std::vector<double> dd(LARGE_ARRAY_SIZE);
                std::vector<std::string> stringVector{"イロハニホヘト", "チリヌルヲ", "ワカヨタレソ"};

                TestNamedPortable portableArray[5];
                std::vector<TestNamedPortable> nn;
                for (int i = 0; i < 5; i++) {
                    nn.emplace_back(TestNamedPortable{"named-portable-" + std::to_string(i), i});
                }
                TestInnerPortable inner{bb, ba, cc, ss, ii, ll, ff, dd, stringVector, nn};
                data = serializationService.toData<TestInnerPortable>(inner);
                auto tip1 = serializationService.toObject<TestInnerPortable>(data);
                auto tip2 = serializationService.toObject<TestInnerPortable>(data);
                ASSERT_TRUE(tip1);
                ASSERT_EQ(inner, *tip1);
                ASSERT_TRUE(tip2);
                ASSERT_EQ(inner, *tip2);
            }

            TEST_F(ClientSerializationTest, testBasicFunctionalityWithDifferentVersions) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig2.setPortableVersion(2);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);
                serialization::pimpl::Data data;

                int32_t x = 3;
                data = serializationService.toData<int32_t>(&x);
                auto returnedInt = serializationService.toObject<int32_t>(data);
                ASSERT_EQ(x, *returnedInt);

                int16_t f = 2;
                data = serializationService.toData<int16_t>(&f);

                auto temp = serializationService.toObject<int16_t>(data);
                ASSERT_EQ(f, *temp);

                TestNamedPortable np{"name", 5};
                data = serializationService.toData<TestNamedPortable>(&np);
                auto tnp1 = serializationService.toObject<TestNamedPortable>(data);
                auto tnp2 = serializationService2.toObject<TestNamedPortable>(data);
                ASSERT_TRUE(tnp1);
                ASSERT_TRUE(tnp2);
                ASSERT_EQ(np, *tnp1);
                ASSERT_EQ(np, *tnp2);

                TestInnerPortable inner = createInnerPortable();
                data = serializationService.toData<TestInnerPortable>(&inner);

                auto tip1 = serializationService.toObject<TestInnerPortable>(data);
                auto tip2 = serializationService2.toObject<TestInnerPortable>(data);
                ASSERT_TRUE(tip1);
                ASSERT_TRUE(tip2);
                ASSERT_EQ(inner, *tip1);
                ASSERT_EQ(inner, *tip2);

                TestMainPortable main{(byte) 113, true, 'x', -500, 56789, -50992225, 900.5678f, -897543.3678909,
                                      "this is main portable object created for testing!", inner};
                data = serializationService.toData<TestMainPortable>(&main);

                auto tmp1 = serializationService.toObject<TestMainPortable>(data);
                auto tmp2 = serializationService2.toObject<TestMainPortable>(data);
                ASSERT_TRUE(tmp1);
                ASSERT_TRUE(tmp2);
                ASSERT_EQ(main, *tmp1);
                ASSERT_EQ(main, *tmp2);
            }

            TEST_F(ClientSerializationTest, testDataHash) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                std::string serializable = "key1";
                serialization::pimpl::Data data = serializationService.toData<std::string>(&serializable);
                serialization::pimpl::Data data2 = serializationService.toData<std::string>(&serializable);
                ASSERT_EQ(data.getPartitionHash(), data2.getPartitionHash());

            }

            TEST_F(ClientSerializationTest, testPrimitives) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);
                byte by = 2;
                bool boolean = true;
                char c = 'c';
                int16_t s = 4;
                int32_t i = 2000;
                int64_t l = 321324141;
                float f = 3.14f;
                double d = 3.14334;
                std::string str = "Hello world";
                std::string utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                ASSERT_EQ(by, toDataAndBackToObject(serializationService, by));
                ASSERT_EQ(boolean, toDataAndBackToObject(serializationService, boolean));
                ASSERT_EQ(c, toDataAndBackToObject(serializationService, c));
                ASSERT_EQ(s, toDataAndBackToObject(serializationService, s));
                ASSERT_EQ(i, toDataAndBackToObject(serializationService, i));
                ASSERT_EQ(l, toDataAndBackToObject(serializationService, l));
                ASSERT_EQ(f, toDataAndBackToObject(serializationService, f));
                ASSERT_EQ(d, toDataAndBackToObject(serializationService, d));
                ASSERT_EQ(str, toDataAndBackToObject(serializationService, str));
                ASSERT_EQ(utfStr, toDataAndBackToObject(serializationService, utfStr));
            }

            TEST_F(ClientSerializationTest, testPrimitiveArrays) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                char charArray[] = {'c', 'h', 'a', 'r'};
                std::vector<char> cc(charArray, charArray + 4);
                bool boolArray[] = {true, false, false, true};
                byte byteArray[] = {0, 1, 2};
                std::vector<byte> bb(byteArray, byteArray + 3);
                std::vector<bool> ba(boolArray, boolArray + 4);
                int16_t shortArray[] = {3, 4, 5};
                std::vector<int16_t> ss(shortArray, shortArray + 3);
                int32_t integerArray[] = {9, 8, 7, 6};
                std::vector<int32_t> ii(integerArray, integerArray + 4);
                int64_t longArray[] = {0, 1, 5, 7, 9, 11};
                std::vector<int64_t> ll(longArray, longArray + 6);
                float floatArray[] = {0.6543f, -3.56f, 45.67f};
                std::vector<float> ff(floatArray, floatArray + 3);
                double doubleArray[] = {456.456, 789.789, 321.321};
                std::vector<double> dd(doubleArray, doubleArray + 3);
                const std::string stringArray[] = {"ali", "veli", "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"};
                std::vector<std::string> stringVector;
                for (int i = 0; i < 3; ++i) {
                    stringVector.push_back(stringArray[i]);
                }

                ASSERT_EQ(cc, toDataAndBackToObject<std::vector<char> >(serializationService, cc));
                ASSERT_EQ(ba, toDataAndBackToObject<std::vector<bool> >(serializationService, ba));
                ASSERT_EQ(bb, toDataAndBackToObject<std::vector<byte> >(serializationService, bb));
                ASSERT_EQ(ss, toDataAndBackToObject<std::vector<int16_t> >(serializationService, ss));
                ASSERT_EQ(ii, toDataAndBackToObject<std::vector<int32_t> >(serializationService, ii));
                ASSERT_EQ(ll, toDataAndBackToObject<std::vector<int64_t> >(serializationService, ll));
                ASSERT_EQ(ff, toDataAndBackToObject<std::vector<float> >(serializationService, ff));
                ASSERT_EQ(dd, toDataAndBackToObject<std::vector<double> >(serializationService, dd));
                ASSERT_EQ(stringVector, toDataAndBackToObject<std::vector<std::string> >(serializationService,
                                                                                         stringVector));
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithPortable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ObjectCarryingPortable<TestNamedPortable> objectCarryingPortable{TestNamedPortable{"name", 2}};
                serialization::pimpl::Data data = ss.toData < ObjectCarryingPortable < TestNamedPortable > > (
                        &objectCarryingPortable);
                auto ptr = ss.toObject < ObjectCarryingPortable < TestNamedPortable > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithIdentifiedDataSerializable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ObjectCarryingPortable <TestDataSerializable> objectCarryingPortable{TestDataSerializable{2, 'c'}};
                serialization::pimpl::Data data = ss.toData < ObjectCarryingPortable < TestDataSerializable > > (
                        &objectCarryingPortable);
                auto ptr = ss.toObject < ObjectCarryingPortable < TestDataSerializable > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithCustomXSerializable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                ObjectCarryingPortable <TestCustomXSerializable> objectCarryingPortable{TestCustomXSerializable{131321}};
                serialization::pimpl::Data data = ss.toData < ObjectCarryingPortable < TestCustomXSerializable > > (
                        &objectCarryingPortable);
                auto ptr = ss.toObject < ObjectCarryingPortable < TestCustomXSerializable > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }

            TEST_F(ClientSerializationTest, testWriteObjectWithCustomPersonSerializable) {
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);

                ObjectCarryingPortable <TestCustomPerson> objectCarryingPortable{TestCustomPerson{"TestCustomPerson"}};
                serialization::pimpl::Data data = ss.toData < ObjectCarryingPortable < TestCustomPerson > > (
                        &objectCarryingPortable);
                auto ptr = ss.toObject < ObjectCarryingPortable < TestCustomPerson > > (data);
                ASSERT_EQ(objectCarryingPortable, *ptr);
            }


            TEST_F(ClientSerializationTest, testNullData) {
                serialization::pimpl::Data data;
                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService ss(serializationConfig);
                auto ptr = ss.toObject<int32_t>(data);
                ASSERT_FALSE(ptr.has_value());
            }

            TEST_F(ClientSerializationTest, testMorphingPortableV1ToV2Conversion) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestNamedPortable p{"portable-v1", 123};
                serialization::pimpl::Data data = serializationService.toData<TestNamedPortable>(p);

                auto p2 = serializationService.toObject<TestNamedPortableV2>(data);
                ASSERT_TRUE(p2);
                ASSERT_EQ(std::string("portable-v1"), p2->name);
                ASSERT_EQ(123, p2->k);
                ASSERT_EQ(0, p2->v);
            }

            TEST_F(ClientSerializationTest, testMorphingPortableV2ToV1Conversion) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestNamedPortableV2 p2{"portable-v2", 123, 9999};
                serialization::pimpl::Data data = serializationService.toData<TestNamedPortableV2>(p2);

                auto p = serializationService.toObject<TestNamedPortable>(data);
                ASSERT_TRUE(p);
                ASSERT_EQ(std::string("portable-v2"), p->name);
                ASSERT_EQ(123, p->k);
            }

            TEST_F(ClientSerializationTest, testMorphingPortableV1ToV3Conversion) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                TestNamedPortable p{"portable-v1", 123};
                serialization::pimpl::Data data = serializationService.toData<TestNamedPortable>(p);

                ASSERT_THROW(serializationService.toObject<TestNamedPortableV3>(data), exception::HazelcastSerializationException);
            }

            TEST_F(ClientSerializationTest, testMorphingPortableWithDifferentTypes_differentVersions_V2ToV1) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                SerializationConfig serializationConfig2;
                serializationConfig.setPortableVersion(5);
                serialization::pimpl::SerializationService serializationService2(serializationConfig2);

                TestNamedPortableV2 p2{"portable-v2", 123, 7};
                serialization::pimpl::Data data2 = serializationService2.toData<TestNamedPortableV2>(p2);

                auto t1 = serializationService.toObject<TestNamedPortable>(data2);
                ASSERT_TRUE(t1.has_value());
                ASSERT_EQ(std::string("portable-v2"), t1->name);
                ASSERT_EQ(123, t1->k);
            }

            TEST_F(ClientSerializationTest, ObjectDataInputOutput) {
                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization::ObjectDataOutput out;

                byte by = 2;
                bool boolean = true;
                char c = 'c';
                int16_t s = 4;
                int32_t i = 2000;
                int64_t l = 321324141;
                float f = 3.14f;
                double d = 3.14334;
                std::string str = "Hello world";
                std::string utfStr = "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム";

                std::vector<byte> byteVec = {50, 100, 150, 200};
                std::vector<char> cc = {'c', 'h', 'a', 'r'};
                std::vector<bool> ba = {true, false, false, true};
                std::vector<int16_t> ss = {3, 4, 5};
                std::vector<int32_t> ii = {9, 8, 7, 6};
                std::vector<int64_t> ll = {0, 1, 5, 7, 9, 11};
                std::vector<float> ff = {0.6543f, -3.56f, 45.67f};
                std::vector<double> dd = {456.456, 789.789, 321.321};
                std::vector<std::string> stringVector = {"ali", "veli", "イロハニホヘト チリヌルヲ ワカヨタレソ ツネナラム"};

                out.write<byte>(by);
                out.write(c);
                out.write(boolean);
                out.write<int16_t>(s);
                out.write<int32_t>(i);
                out.write<int64_t>(l);
                out.write<float>(f);
                out.write<double>(d);
                out.write(str);
                out.write(utfStr);

                out.write(&byteVec);
                out.write(&cc);
                out.write(&ba);
                out.write(&ss);
                out.write(&ii);
                out.write(&ff);
                out.write(&dd);
                out.write(&stringVector);

                out.writeObject<byte>(&by);
                out.writeObject<char>(c);
                out.writeObject<bool>(&boolean);
                out.writeObject<int16_t>(&s);
                out.writeObject<int32_t>(&i);
                out.writeObject<float>(&f);
                out.writeObject<double>(&d);
                out.writeObject<std::string>(&str);
                out.writeObject<std::string>(&utfStr);
                out.write<int32_t>(5);
                out.write<std::string>(nullptr);
                out.write<std::vector<std::string>>(nullptr);

                serialization::ObjectDataInput in(out.toByteArray(), 0, serializationService.getPortableSerializer(), serializationService.getDataSerializer(),
                                                  serializationConfig.getGlobalSerializer());

                ASSERT_EQ(by, in.read<byte>());
                ASSERT_EQ(c, in.read<char>());
                ASSERT_EQ(boolean, in.read<bool>());
                ASSERT_EQ(s, in.read<int16_t>());
                ASSERT_EQ(i, in.read<int32_t>());
                ASSERT_EQ(l, in.read<int64_t>());
                ASSERT_FLOAT_EQ(f, in.read<float>());
                ASSERT_DOUBLE_EQ(d, in.read<double>());
                ASSERT_EQ(str, in.read<std::string>());
                ASSERT_EQ(utfStr, in.read<std::string>());

                ASSERT_EQ(byteVec, *in.read<std::vector<byte>>());
                ASSERT_EQ(cc, *in.read<std::vector<char>>());
                ASSERT_EQ(ba, *in.read<std::vector<bool>>());
                ASSERT_EQ(ss, *in.read<std::vector<int16_t>>());
                ASSERT_EQ(ii, *in.read<std::vector<int32_t>>());
                ASSERT_EQ(ff, *in.read<std::vector<float>>());
                ASSERT_EQ(dd, *in.read<std::vector<double>>());
                auto strArrRead = in.read<std::vector<std::string>>();
                ASSERT_TRUE(strArrRead.has_value());
                ASSERT_EQ(stringVector.size(), strArrRead->size());
                for (size_t j = 0; j < stringVector.size(); ++j) {
                    ASSERT_EQ((*strArrRead)[j], stringVector[j]);
                }

                ASSERT_EQ(by, *in.readObject<byte>());
                ASSERT_EQ(c, *in.readObject<char>());
                ASSERT_EQ(boolean, *in.readObject<bool>());
                ASSERT_EQ(s, *in.readObject<int16_t>());
                ASSERT_EQ(i, *in.readObject<int32_t>());
                ASSERT_EQ(f, *in.readObject<float>());
                ASSERT_EQ(d, *in.readObject<double>());
                ASSERT_EQ(str, *in.readObject<std::string>());
                ASSERT_EQ(utfStr, *in.readObject<std::string>());
                ASSERT_EQ(4, in.skipBytes(4));
                ASSERT_FALSE(in.read<boost::optional<std::string>>().has_value()) << "Expected null string";
                ASSERT_FALSE(in.read<std::vector<std::string>>().has_value()) << "Expected null string array";
            }

            TEST_F(ClientSerializationTest, testGetUTF8CharCount) {
                std::string utfStr = "xyzä123";

                SerializationConfig serializationConfig;
                serializationConfig.setPortableVersion(1);
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization::ObjectDataOutput out;

                out.write(utfStr);
                auto byteArray = out.toByteArray();
                int strLen = hazelcast::util::Bits::readIntB(byteArray, 0);
                ASSERT_EQ(7, strLen);
            }

            TEST_F(ClientSerializationTest, testExtendedAscii) {
                std::string utfStr = "Num\xc3\xa9ro";

                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                serialization::pimpl::Data data = serializationService.toData<std::string>(&utfStr);
                auto deserializedString = serializationService.toObject<std::string>(data);
                ASSERT_TRUE(deserializedString.has_value());
                ASSERT_EQ(utfStr, deserializedString.value());
            }

            TEST_F(ClientSerializationTest, testExtendedAsciiIncorrectUtf8Write) {
                std::string utfStr = "Num\351ro";

                SerializationConfig serializationConfig;
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                ASSERT_THROW(serializationService.toData<std::string>(&utfStr), exception::UTFDataFormatException);
            }

            TEST_F(ClientSerializationTest, testGlobalSerializer) {
                SerializationConfig serializationConfig;
                serializationConfig.setGlobalSerializer(std::make_shared<DummyGlobalSerializer>());
                serialization::pimpl::SerializationService serializationService(serializationConfig);

                NonSerializableObject obj{"My class with no serializer"};

                serialization::pimpl::Data data = serializationService.toData(obj);

                auto deserializedValue = serializationService.toObject<NonSerializableObject>(data);
                ASSERT_TRUE(deserializedValue.has_value());
                ASSERT_EQ(obj, deserializedValue.value());
            }
        }
    }
}

namespace hazelcast {
    namespace client {
        namespace test {
            namespace internal {
                namespace nearcache {
                    class NearCacheRecordStoreTest
                            : public ClientTestSupport, public ::testing::WithParamInterface<config::InMemoryFormat> {
                    public:
                        NearCacheRecordStoreTest() {
                            ss = std::unique_ptr<serialization::pimpl::SerializationService>(
                                    new serialization::pimpl::SerializationService(serializationConfig));
                        }

                    protected:
                        static const int DEFAULT_RECORD_COUNT;
                        static const char *DEFAULT_NEAR_CACHE_NAME;

                        void putAndGetRecord(config::InMemoryFormat inMemoryFormat) {
                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            ASSERT_EQ(DEFAULT_RECORD_COUNT, nearCacheRecordStore->size());

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                auto value = nearCacheRecordStore->get(getSharedKey(i));
                                ASSERT_TRUE(value);
                                ASSERT_EQ(*getSharedValue(i), *value);
                            }
                        }

                        void putAndRemoveRecord(config::InMemoryFormat inMemoryFormat) {
                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                std::shared_ptr<serialization::pimpl::Data> key = getSharedKey(i);
                                nearCacheRecordStore->put(key, getSharedValue(i));

                                // ensure that they are stored
                                ASSERT_TRUE(nearCacheRecordStore->get(key));
                            }

                            ASSERT_EQ(DEFAULT_RECORD_COUNT, nearCacheRecordStore->size());

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                std::shared_ptr<serialization::pimpl::Data> key = getSharedKey(i);
                                ASSERT_TRUE(nearCacheRecordStore->invalidate(key));
                                ASSERT_FALSE(nearCacheRecordStore->get(key)) << "Should not exist";
                            }

                            ASSERT_EQ(0, nearCacheRecordStore->size());
                        }

                        void clearRecordsOrDestroyStore(config::InMemoryFormat inMemoryFormat, bool destroy) {
                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                std::shared_ptr<serialization::pimpl::Data> key = getSharedKey(i);
                                nearCacheRecordStore->put(key, getSharedValue(i));

                                // ensure that they are stored
                                ASSERT_TRUE(nearCacheRecordStore->get(key));
                            }

                            if (destroy) {
                                nearCacheRecordStore->destroy();
                            } else {
                                nearCacheRecordStore->clear();
                            }

                            ASSERT_EQ(0, nearCacheRecordStore->size());
                        }

                        void statsCalculated(config::InMemoryFormat inMemoryFormat) {
                            int64_t creationStartTime = hazelcast::util::currentTimeMillis();
                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);
                            int64_t creationEndTime = hazelcast::util::currentTimeMillis();

                            int64_t expectedEntryCount = 0;
                            int64_t expectedHits = 0;
                            int64_t expectedMisses = 0;

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));

                                expectedEntryCount++;
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                int selectedKey = i * 3;
                                if (nearCacheRecordStore->get(getSharedKey(selectedKey)) != nullptr) {
                                    expectedHits++;
                                } else {
                                    expectedMisses++;
                                }
                            }

                            auto nearCacheStats = nearCacheRecordStore->getNearCacheStats();

                            int64_t memoryCostWhenFull = nearCacheStats->getOwnedEntryMemoryCost();
                            ASSERT_TRUE(nearCacheStats->getCreationTime() >= creationStartTime);
                            ASSERT_TRUE(nearCacheStats->getCreationTime() <= creationEndTime);
                            ASSERT_EQ(expectedHits, nearCacheStats->getHits());
                            ASSERT_EQ(expectedMisses, nearCacheStats->getMisses());
                            ASSERT_EQ(expectedEntryCount, nearCacheStats->getOwnedEntryCount());
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    ASSERT_TRUE(memoryCostWhenFull > 0);
                                    break;
                                case config::OBJECT:
                                    ASSERT_EQ(0, memoryCostWhenFull);
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                int selectedKey = i * 3;
                                if (nearCacheRecordStore->invalidate(getSharedKey(selectedKey))) {
                                    expectedEntryCount--;
                                }
                            }

                            ASSERT_EQ(expectedEntryCount, nearCacheStats->getOwnedEntryCount());
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    ASSERT_TRUE(nearCacheStats->getOwnedEntryMemoryCost() > 0);
                                    ASSERT_TRUE(nearCacheStats->getOwnedEntryMemoryCost() < memoryCostWhenFull);
                                    break;
                                case config::OBJECT:
                                    ASSERT_EQ(0, nearCacheStats->getOwnedEntryMemoryCost());
                                    break;
                            }

                            nearCacheRecordStore->clear();

                            switch (inMemoryFormat) {
                                case config::BINARY:
                                case config::OBJECT:
                                    ASSERT_EQ(0, nearCacheStats->getOwnedEntryMemoryCost());
                                    break;
                            }
                        }

                        void ttlEvaluated(config::InMemoryFormat inMemoryFormat) {
                            int ttlSeconds = 3;

                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            nearCacheConfig.setTimeToLiveSeconds(ttlSeconds);

                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_TRUE(nearCacheRecordStore->get(getSharedKey(i)));
                            }

                            hazelcast::util::sleep(ttlSeconds + 1);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_FALSE(nearCacheRecordStore->get(getSharedKey(i)));
                            }
                        }

                        void maxIdleTimeEvaluatedSuccessfully(config::InMemoryFormat inMemoryFormat) {
                            int maxIdleSeconds = 3;

                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            nearCacheConfig.setMaxIdleSeconds(maxIdleSeconds);

                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_TRUE(nearCacheRecordStore->get(getSharedKey(i)));
                            }

                            hazelcast::util::sleep(maxIdleSeconds + 1);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                ASSERT_FALSE(nearCacheRecordStore->get(getSharedKey(i)));
                            }
                        }

                        void expiredRecordsCleanedUpSuccessfully(config::InMemoryFormat inMemoryFormat,
                                                                 bool useIdleTime) {
                            int cleanUpThresholdSeconds = 3;

                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            if (useIdleTime) {
                                nearCacheConfig.setMaxIdleSeconds(cleanUpThresholdSeconds);
                            } else {
                                nearCacheConfig.setTimeToLiveSeconds(cleanUpThresholdSeconds);
                            }

                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                            }

                            hazelcast::util::sleep(cleanUpThresholdSeconds + 1);

                            nearCacheRecordStore->doExpiration();

                            ASSERT_EQ(0, nearCacheRecordStore->size());

                            auto nearCacheStats = nearCacheRecordStore->getNearCacheStats();
                            ASSERT_EQ(0, nearCacheStats->getOwnedEntryCount());
                            ASSERT_EQ(0, nearCacheStats->getOwnedEntryMemoryCost());
                        }

                        void createNearCacheWithMaxSizePolicy(config::InMemoryFormat inMemoryFormat,
                                                              config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>::MaxSizePolicy maxSizePolicy,
                                                              int32_t size) {
                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);
                            std::shared_ptr<config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data> > evictionConfig(
                                    new config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>());
                            evictionConfig->setMaximumSizePolicy(maxSizePolicy);
                            evictionConfig->setSize(size);
                            nearCacheConfig.setEvictionConfig(evictionConfig);

                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);
                        }

                        void doEvictionWithEntryCountMaxSizePolicy(config::InMemoryFormat inMemoryFormat,
                                                                   config::EvictionPolicy evictionPolicy) {
                            int32_t maxSize = DEFAULT_RECORD_COUNT / 2;

                            auto nearCacheConfig = createNearCacheConfig(
                                    DEFAULT_NEAR_CACHE_NAME, inMemoryFormat);

                            std::shared_ptr<config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data> > evictionConfig(
                                    new config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>());

                            evictionConfig->setMaximumSizePolicy(config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>::ENTRY_COUNT);
                            evictionConfig->setSize(maxSize);
                            evictionConfig->setEvictionPolicy(evictionPolicy);
                            nearCacheConfig.setEvictionConfig(evictionConfig);

                            auto nearCacheRecordStore = createNearCacheRecordStore(nearCacheConfig, inMemoryFormat);

                            for (int i = 0; i < DEFAULT_RECORD_COUNT; i++) {
                                nearCacheRecordStore->put(getSharedKey(i), getSharedValue(i));
                                nearCacheRecordStore->doEvictionIfRequired();
                                ASSERT_TRUE(maxSize >= nearCacheRecordStore->size());
                            }
                        }

                        std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, serialization::pimpl::Data> > createNearCacheRecordStore(
                                config::NearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data> &nearCacheConfig,
                                config::InMemoryFormat inMemoryFormat) {
                            std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, serialization::pimpl::Data> > recordStore;
                            switch (inMemoryFormat) {
                                case config::BINARY:
                                    recordStore = std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, serialization::pimpl::Data> >(
                                            new hazelcast::client::internal::nearcache::impl::store::NearCacheDataRecordStore<serialization::pimpl::Data, serialization::pimpl::Data, serialization::pimpl::Data>(
                                                    DEFAULT_NEAR_CACHE_NAME, nearCacheConfig, *ss));
                                    break;
                                case config::OBJECT:
                                    recordStore = std::unique_ptr<hazelcast::client::internal::nearcache::impl::NearCacheRecordStore<serialization::pimpl::Data, serialization::pimpl::Data> >(
                                            new hazelcast::client::internal::nearcache::impl::store::NearCacheObjectRecordStore<serialization::pimpl::Data, serialization::pimpl::Data, serialization::pimpl::Data>(
                                                    DEFAULT_NEAR_CACHE_NAME,
                                                    nearCacheConfig, *ss));
                                    break;
                                default:
                                    std::ostringstream out;
                                    out << "Unsupported in-memory format: " << inMemoryFormat;
                                    BOOST_THROW_EXCEPTION(
                                            exception::IllegalArgumentException("NearCacheRecordStoreTest", out.str()));
                            }
                            recordStore->initialize();

                            return recordStore;
                        }

                        config::NearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data>
                        createNearCacheConfig(const char *name,
                                              config::InMemoryFormat inMemoryFormat) {
                            config::NearCacheConfig<serialization::pimpl::Data, serialization::pimpl::Data> config;
                            config.setName(name).setInMemoryFormat(inMemoryFormat);
                            return config;
                        }

                        std::shared_ptr<serialization::pimpl::Data> getSharedValue(int value) const {
                            char buf[30];
                            hazelcast::util::hz_snprintf(buf, 30, "Record-%ld", value);
                            return ss->toSharedData(new std::string(buf));
                        }

                        std::shared_ptr<serialization::pimpl::Data> getSharedKey(int value) {
                            return ss->toSharedData<int>(&value);
                        }

                        std::unique_ptr<serialization::pimpl::SerializationService> ss;
                        SerializationConfig serializationConfig;
                    };

                    const int NearCacheRecordStoreTest::DEFAULT_RECORD_COUNT = 100;
                    const char *NearCacheRecordStoreTest::DEFAULT_NEAR_CACHE_NAME = "TestNearCache";

                    TEST_P(NearCacheRecordStoreTest, putAndGetRecord) {
                        putAndGetRecord(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, putAndRemoveRecord) {
                        putAndRemoveRecord(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, clearRecords) {
                        clearRecordsOrDestroyStore(GetParam(), false);
                    }

                    TEST_P(NearCacheRecordStoreTest, destroyStore) {
                        clearRecordsOrDestroyStore(GetParam(), true);
                    }

                    TEST_P(NearCacheRecordStoreTest, statsCalculated) {
                        statsCalculated(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, ttlEvaluated) {
                        ttlEvaluated(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, maxIdleTimeEvaluatedSuccessfully) {
                        maxIdleTimeEvaluatedSuccessfully(GetParam());
                    }

                    TEST_P(NearCacheRecordStoreTest, expiredRecordsCleanedUpSuccessfullyBecauseOfTTL) {
                        expiredRecordsCleanedUpSuccessfully(GetParam(), false);
                    }

                    TEST_P(NearCacheRecordStoreTest, expiredRecordsCleanedUpSuccessfullyBecauseOfIdleTime) {
                        expiredRecordsCleanedUpSuccessfully(GetParam(), true);
                    }

                    TEST_P(NearCacheRecordStoreTest, canCreateWithEntryCountMaxSizePolicy) {
                        createNearCacheWithMaxSizePolicy(GetParam(),
                                                         config::EvictionConfig<serialization::pimpl::Data, serialization::pimpl::Data>::ENTRY_COUNT,
                                                         1000);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndLRUEvictionPolicy) {
                        doEvictionWithEntryCountMaxSizePolicy(GetParam(), config::LRU);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndLFUEvictionPolicy) {
                        doEvictionWithEntryCountMaxSizePolicy(GetParam(), config::LFU);
                    }

                    TEST_P(NearCacheRecordStoreTest,
                           evictionTriggeredAndHandledSuccessfullyWithEntryCountMaxSizePolicyAndRandomEvictionPolicy) {
                        doEvictionWithEntryCountMaxSizePolicy(GetParam(), config::RANDOM);
                    }

                    INSTANTIATE_TEST_SUITE_P(BasicStoreTest, NearCacheRecordStoreTest,
                                             ::testing::Values(config::BINARY, config::OBJECT));

                }
            }
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

