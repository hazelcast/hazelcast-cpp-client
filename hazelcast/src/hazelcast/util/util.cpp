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

#include <cmath>
#include <cassert>
#include <cerrno>
#include <cstdlib>
#include <climits>
#include <limits>
#include <iosfwd>
#include <string.h>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <thread>
#include <regex>
#include <iomanip>
#include <mutex>
#include <stdlib.h>
#include <time.h>

#include <boost/concept_check.hpp>

#ifdef HZ_BUILD_WITH_SSL
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/asio/ssl/rfc2818_verification.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/system_error.hpp>
#endif // HZ_BUILD_WITH_SSL

#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/AddressUtil.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/SyncHttpsClient.h"
#include "hazelcast/util/Clearable.h"
#include "hazelcast/util/hz_thread_pool.h"
#include <mutex>

#include "hazelcast/util/Destroyable.h"
#include "hazelcast/util/TimeUtil.h"
#include "hazelcast/util/concurrent/TimeUnit.h"
#include "hazelcast/util/Closeable.h"
#include "hazelcast/util/Runnable.h"
#include "hazelcast/util/UUID.h"
#include "hazelcast/util/UTFUtil.h"
#include "hazelcast/util/SyncHttpClient.h"
#include "hazelcast/util/AtomicBoolean.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/util/concurrent/BackoffIdleStrategy.h"
#include "hazelcast/util/concurrent/CancellationException.h"
#include "hazelcast/util/UuidUtil.h"
#include "hazelcast/util/AtomicInt.h"
#include "hazelcast/util/AddressHelper.h"
#include "hazelcast/util/RuntimeAvailableProcessors.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/exception/ProtocolExceptions.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/ExceptionUtil.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4996) //for strerror
#endif

namespace hazelcast {
    namespace util {

        AddressHolder AddressUtil::getAddressHolder(const std::string &address, int defaultPort) {
            int indexBracketStart = static_cast<int>(address.find('['));
            int indexBracketEnd = static_cast<int>(address.find(']', indexBracketStart));
            int indexColon = static_cast<int>(address.find(':'));
            int lastIndexColon = static_cast<int>(address.rfind(':'));
            std::string host;
            int port = defaultPort;
            std::string scopeId;
            if (indexColon > -1 && lastIndexColon > indexColon) {
                // IPv6
                if (indexBracketStart == 0 && indexBracketEnd > indexBracketStart) {
                    host = address.substr(indexBracketStart + 1, indexBracketEnd - (indexBracketStart + 1));
                    if (lastIndexColon == indexBracketEnd + 1) {
                        port = atoi(address.substr(lastIndexColon + 1).c_str());
                    }
                } else {
                    host = address;
                }
                int indexPercent = static_cast<int>(host.find('%'));
                if (indexPercent != -1) {
                    scopeId = host.substr(indexPercent + 1);
                    host = host.substr(0, indexPercent);
                }
            } else if (indexColon > 0 && indexColon == lastIndexColon) {
                host = address.substr(0, indexColon);
                port = atoi(address.substr(indexColon + 1).c_str());
            } else {
                host = address;
            }
            return AddressHolder(host, scopeId, port);
        }

        AddressHolder AddressUtil::getAddressHolder(const std::string &address) {
            return getAddressHolder(address, -1);
        }

        boost::asio::ip::address AddressUtil::getByName(const std::string &host) {
            return getByName(host, "");
        }

        boost::asio::ip::address AddressUtil::getByName(const std::string &host, const std::string &service) {
            try {
                boost::asio::io_service ioService;
                boost::asio::ip::tcp::resolver res(ioService);
                boost::asio::ip::tcp::resolver::query query(host, service);
                boost::asio::ip::basic_resolver<boost::asio::ip::tcp>::iterator iterator = res.resolve(query);
                return iterator->endpoint().address();
            } catch (boost::system::system_error &e) {
                std::ostringstream out;
                out << "Address " << host << " ip number is not available. " << e.what();
                throw client::exception::UnknownHostException("AddressUtil::getByName", out.str());
            }
        }

    }
}

namespace hazelcast {
    namespace util {
        SyncHttpsClient::SyncHttpsClient(const std::string &serverIp, const std::string &uriPath) : server(serverIp),
                                                                                                    uriPath(uriPath),
#ifdef HZ_BUILD_WITH_SSL
                                                                                                    sslContext(
                                                                                                            boost::asio::ssl::context::sslv23),
#endif
                                                                                                    responseStream(
                                                                                                            &response) {
            util::Preconditions::checkSSL("SyncHttpsClient::SyncHttpsClient");

#ifdef HZ_BUILD_WITH_SSL
            sslContext.set_default_verify_paths();
            sslContext.set_options(boost::asio::ssl::context::default_workarounds | boost::asio::ssl::context::no_sslv2 |
                                   boost::asio::ssl::context::single_dh_use);

            socket = std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket> >(
                    new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(ioService, sslContext));
#endif // HZ_BUILD_WITH_SSL
        }

        std::istream &SyncHttpsClient::openConnection() {
            util::Preconditions::checkSSL("SyncHttpsClient::openConnection");

#ifdef HZ_BUILD_WITH_SSL
            try {
                // Get a list of endpoints corresponding to the server name.
                boost::asio::ip::tcp::resolver resolver(ioService);
                boost::asio::ip::tcp::resolver::query query(server, "https");
                boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

                boost::asio::connect(socket->lowest_layer(), endpoint_iterator);

                socket->lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));

                socket->set_verify_callback(boost::asio::ssl::rfc2818_verification(server));
                socket->handshake(boost::asio::ssl::stream_base::client);

                // Form the request. We specify the "Connection: close" header so that the
                // server will close the socket after transmitting the response. This will
                // allow us to treat all data up until the EOF as the content.
                boost::asio::streambuf request;
                std::ostream request_stream(&request);
                request_stream << "GET " << uriPath << " HTTP/1.0\r\n";
                request_stream << "Host: " << server << "\r\n";
                request_stream << "Accept: */*\r\n";
                request_stream << "Connection: close\r\n\r\n";

                // Send the request.
                boost::asio::write(*socket, request.data());

                // Read the response status line. The response streambuf will automatically
                // grow to accommodate the entire line. The growth may be limited by passing
                // a maximum size to the streambuf constructor.
                boost::asio::read_until(*socket, response, "\r\n");

                // Check that response is OK.
                std::string httpVersion;
                responseStream >> httpVersion;
                unsigned int statusCode;
                responseStream >> statusCode;
                std::string statusMessage;
                std::getline(responseStream, statusMessage);
                if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
                    throw client::exception::IOException("openConnection", "Invalid response");
                }
                if (statusCode != 200) {
                    std::stringstream out;
                    out << "Response returned with status: " << statusCode << " Status message:" << statusMessage;
                    throw client::exception::IOException("SyncHttpsClient::openConnection", out.str());;
                }

                // Read the response headers, which are terminated by a blank line.
                boost::asio::read_until(*socket, response, "\r\n\r\n");

                // Process the response headers.
                std::string header;
                while (std::getline(responseStream, header) && header != "\r");

                // Read until EOF
                boost::system::error_code error;
                size_t bytesRead;
                while ((bytesRead = boost::asio::read(*socket, response.prepare(1024),
                                               boost::asio::transfer_at_least(1), error))) {
                    response.commit(bytesRead);
                }

                if (error != boost::asio::error::eof) {
                    throw boost::system::system_error(error);
                }
            } catch (boost::system::system_error &e) {
                std::ostringstream out;
                out << "Could not retrieve response from https://" << server << uriPath << " Error:" << e.what();
                throw client::exception::IOException("SyncHttpsClient::openConnection", out.str());
            }
#endif // HZ_BUILD_WITH_SSL

            return responseStream;
        }
    }
}


namespace hazelcast {
    namespace util {
        Clearable::~Clearable() {
        }
    }
}


//
// Created by ihsan demir on 9 Dec 2016.



namespace hazelcast {
    namespace util {
        int HashUtil::hashToIndex(int hash, int length) {
            Preconditions::checkPositive(length, "mod must be larger than 0");

            if (hash == INT_MIN) {
                hash = 0;
            } else {
                hash = std::abs(hash);
            }

            return hash % length;
        }
    }
}

namespace hazelcast {
    namespace util {
        Destroyable::~Destroyable() {
        }
    }
}



namespace hazelcast {
    namespace util {
        int64_t TimeUtil::timeInMsOrOneIfResultIsZero(int64_t time, const concurrent::TimeUnit &timeunit) {
            int64_t timeInMillis = timeunit.toMillis(time);
            if (time > 0 && timeInMillis == 0) {
                timeInMillis = 1;
            }

            return timeInMillis;
        }
    }
}

//  Copyright (c) 2015 ihsan demir. All rights reserved.
//


namespace hazelcast {
    namespace util {
        Closeable::~Closeable() {
        }
    }
}



namespace hazelcast {
    namespace util {
        ILogger::ILogger(const std::string &instanceName, const std::string &groupName, const std::string &version,
                         const client::config::LoggerConfig &loggerConfig)
                : instanceName(instanceName), groupName(groupName), version(version), loggerConfig(loggerConfig) {
            std::stringstream out;
            out << instanceName << "[" << groupName << "] [" << HAZELCAST_VERSION << "]";
            prefix = out.str();

            easyLogger = el::Loggers::getLogger(instanceName);
        }

        ILogger::~ILogger() {
        }

        bool ILogger::start() {
            std::string configurationFileName = loggerConfig.getConfigurationFileName();
            if (!configurationFileName.empty()) {
                el::Configurations defaultConf(configurationFileName);
                if (!defaultConf.parseFromFile(configurationFileName)) {
                    return false;
                }
                return el::Loggers::reconfigureLogger(easyLogger, defaultConf) != nullptr;
            }

            el::Configurations defaultConf;

            std::call_once(elOnceflag, el::Loggers::addFlag, el::LoggingFlag::DisableApplicationAbortOnFatalLog);

            defaultConf.set(el::Level::Global, el::ConfigurationType::Format,
                            std::string("%datetime{%d/%M/%Y %h:%m:%s.%g} %level: [%thread] ") + prefix + " %msg");

            defaultConf.set(el::Level::Global, el::ConfigurationType::ToStandardOutput, "true");

            defaultConf.set(el::Level::Global, el::ConfigurationType::ToFile, "false");

            // Disable all levels first and then enable the desired levels
            defaultConf.set(el::Level::Global, el::ConfigurationType::Enabled, "false");

            client::LoggerLevel::Level logLevel = loggerConfig.getLogLevel();
            if (logLevel <= client::LoggerLevel::FINEST) {
                defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::INFO) {
                defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::WARNING) {
                defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "true");
            }
            if (logLevel <= client::LoggerLevel::SEVERE) {
                defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
            }
            return el::Loggers::reconfigureLogger(easyLogger, defaultConf) != nullptr;
        }

        bool ILogger::isEnabled(const client::LoggerLevel::Level &logLevel) const {
            return logLevel >= this->loggerConfig.getLogLevel();
        }

        bool ILogger::isEnabled(int level) const {
            return isEnabled(static_cast<client::LoggerLevel::Level>(level));
        }

        bool ILogger::isFinestEnabled() const {
            return isEnabled(client::LoggerLevel::FINEST);
        }

        const std::string &ILogger::getInstanceName() const {
            return instanceName;
        }
    }
}

namespace hazelcast {
    namespace util {
        bool Runnable::isStriped() {
            return false;
        }

        Runnable::~Runnable() {
        }

        bool StripedRunnable::isStriped() {
            return true;
        }

        RunnableDelegator::RunnableDelegator(Runnable &runnable) : runnable(runnable) {
        }

        void RunnableDelegator::run() {
            runnable.run();
        }

        const std::string RunnableDelegator::getName() const {
            return runnable.getName();
        }

    }
}

//  Copyright (c) 2015 ihsan demir. All rights reserved.
//


namespace hazelcast {
    namespace util {
        UUID::UUID() : mostSigBits(0), leastSigBits(0) {}

        UUID::UUID(int64_t mostBits, int64_t leastBits) : mostSigBits(mostBits), leastSigBits(leastBits) {
        }

        int64_t UUID::getLeastSignificantBits() const {
            return leastSigBits;
        }

        /**
         * Returns the most significant 64 bits of this UUID's 128 bit value.
         *
         * @return the most significant 64 bits of this UUID's 128 bit value.
         */
        int64_t UUID::getMostSignificantBits() const {
            return mostSigBits;
        }

        bool UUID::equals(const UUID &rhs) const {
            return (mostSigBits == rhs.mostSigBits && leastSigBits == rhs.leastSigBits);
        }

        std::string UUID::toString() const {
            return (digits(mostSigBits >> 32, 8) + "-" +
                    digits(mostSigBits >> 16, 4) + "-" +
                    digits(mostSigBits, 4) + "-" +
                    digits(leastSigBits >> 48, 4) + "-" +
                    digits(leastSigBits, 12));
        }

        std::string UUID::digits(int64_t val, int32_t digits) {
            int64_t hi = 1LL << (digits * 4);
            std::ostringstream out;
            out << std::hex << (hi | (val & (hi - 1)));
            std::string value = out.str();
            return value.substr(1);
        }

        bool UUID::operator==(const UUID &rhs) const {
            return this->equals(rhs);
        }

        bool UUID::operator!=(const UUID &rhs) const {
            return !(rhs == *this);
        }

    }
}



namespace hazelcast {
    namespace util {
        int32_t UTFUtil::isValidUTF8(const std::string &str) {
            int32_t numberOfUtf8Chars = 0;
            for (size_t i = 0, len = str.length(); i < len; ++i) {
                unsigned char c = (unsigned char) str[i];
                size_t n = 0;
                // is ascii
                if (c <= 0x7f) {
                    n = 0; // 0bbbbbbb
                } else if ((c & 0xE0) == 0xC0) {
                    n = 1; // 110bbbbb
                } else if (c == 0xed && i < (len - 1) && ((unsigned char) str[i + 1] & 0xa0) == 0xa0) {
                    return -1; //U+d800 to U+dfff
                } else if ((c & 0xF0) == 0xE0) {
                    n = 2; // 1110bbbb
                } else if ((c & 0xF8) == 0xF0) {
                    n = 3; // 11110bbb
                } else {
                    return -1;
                }

                for (size_t j = 0; j < n && i < len; j++) { // n bytes matching 10bbbbbb follow ?
                    if ((++i == len) || (((unsigned char) str[i] & 0xC0) != 0x80)) {
                        return -1;
                    }
                }

                ++numberOfUtf8Chars;
            }

            return numberOfUtf8Chars;
        }

        void UTFUtil::readUTF8Char(UTFUtil::ByteReadable &in, byte firstByte, std::vector<char> &utfBuffer) {
            size_t n = 0;
            // ascii
            if (firstByte <= 0x7f) {
                n = 0; // 0bbbbbbb
            } else if ((firstByte & 0xE0) == 0xC0) {
                n = 1; // 110bbbbb
            } else if ((firstByte & 0xF0) == 0xE0) {
                n = 2; // 1110bbbb
            } else if ((firstByte & 0xF8) == 0xF0) {
                n = 3; // 11110bbb
            } else {
                throw client::exception::UTFDataFormatException("Bits::readUTF8Char", "Malformed byte sequence");
            }

            utfBuffer.push_back((char) firstByte);
            for (size_t j = 0; j < n; j++) {
                byte b = in.readByte();
                if (firstByte == 0xed && (b & 0xa0) == 0xa0) {
                    throw client::exception::UTFDataFormatException("Bits::readUTF8Char",
                                                                    "Malformed byte sequence U+d800 to U+dfff"); //U+d800 to U+dfff
                }

                if ((b & 0xC0) != 0x80) { // n bytes matching 10bbbbbb follow ?
                    throw client::exception::UTFDataFormatException("Bits::readUTF8Char", "Malformed byte sequence");
                }
                utfBuffer.push_back((char) b);
            }
        }

        UTFUtil::ByteReadable::~ByteReadable() {
        }
    }
}

//
// Created by sancar koyunlu on 5/3/13.




namespace hazelcast {
    namespace util {
        int64_t getCurrentThreadId() {
            return std::hash<std::thread::id>{}(std::this_thread::get_id());
        }

        void sleep(int seconds) {
            std::this_thread::sleep_for(std::chrono::seconds(seconds));
        }

        void sleepmillis(uint64_t milliseconds) {
            std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
        }

        int localtime(const time_t *clock, struct tm *result) {
            int returnCode = -1;
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            returnCode = localtime_s(result, clock);
#else
            if (NULL != localtime_r(clock, result)) {
                returnCode = 0;
            }
#endif

            return returnCode;
        }

        int hz_snprintf(char *str, size_t len, const char *format, ...) {
            va_list args;
            va_start(args, format);

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            int result = vsnprintf_s(str, len, _TRUNCATE, format, args);
            if (result < 0) {
                return len > 0 ? len - 1 : 0;
            }
            va_end(args);
            return result;
#else
            int result = vsnprintf(str, len, format, args);
            va_end(args);
            return result;
#endif
        }

        void gitDateToHazelcastLogDate(std::string &date) {
            // convert the date string from "2016-04-20" to 20160420
            date.erase(std::remove(date.begin(), date.end(), '"'), date.end());
            if (date != "NOT_FOUND") {
                date.erase(std::remove(date.begin(), date.end(), '-'), date.end());
            }
        }

        int64_t currentTimeMillis() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        int64_t currentTimeNanos() {
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                    std::chrono::steady_clock::now().time_since_epoch()).count();
        }

        int strerror_s(int errnum, char *strerrbuf, size_t buflen, const char *msgPrefix) {
            int numChars = 0;
            if ((const char *) NULL != msgPrefix) {
                numChars = util::hz_snprintf(strerrbuf, buflen, "%s ", msgPrefix);
                if (numChars < 0) {
                    return numChars;
                }

                if (numChars >= (int) buflen - 1) {
                    return 0;
                }
            }

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,
                  NULL,
                  errnum,
                  0,
                  (LPTSTR)(strerrbuf + numChars),
                  buflen - numChars,
                  NULL)) {
                return -1;
            }
            return 0;
#elif defined(__llvm__) && !_GNU_SOURCE
            /* XSI-compliant */
            return ::strerror_r(errnum, strerrbuf + numChars, buflen - numChars);
#else
            /* GNU-specific */
            char *errStr = ::strerror_r(errnum, strerrbuf + numChars, buflen - numChars);
            int result = util::hz_snprintf(strerrbuf + numChars, buflen - numChars, "%s", errStr);
            if (result < 0) {
                return result;
            }
            return 0;
#endif
        }

        int32_t getAvailableCoreCount() {
            return (int32_t) std::thread::hardware_concurrency();
        }

        std::string StringUtil::timeToString(std::chrono::steady_clock::time_point t) {
            using namespace std::chrono;

            if (!t.time_since_epoch().count()) {
                return std::string("never");
            }

            auto systemDuration = duration_cast<system_clock::duration>(t - steady_clock::now());
            auto brokenTime = system_clock::to_time_t(system_clock::now() + systemDuration);
            struct tm localBrokenTime;
            int result = util::localtime(&brokenTime, &localBrokenTime);
            assert(!result);
            boost::ignore_unused_variable_warning(result);

            std::ostringstream oss;
            oss << std::put_time(&localBrokenTime, "%Y-%m-%d %H:%M:%S");
            oss << '.' << std::setfill('0') << std::setw(3) << duration_cast<milliseconds>(systemDuration).count() % 1000;

            return oss.str();
        }

        std::vector<std::string> StringUtil::tokenizeVersionString(const std::string &version) {
            // passing -1 as the submatch index parameter performs splitting
            std::regex re(".");
            std::sregex_token_iterator first{version.begin(), version.end(), re, -1}, last;
            return {first, last};
        }

        int Int64Util::numberOfLeadingZeros(int64_t i) {
            // HD, Figure 5-6
            if (i == 0)
                return 64;
            int n = 1;
            int64_t x = (int64_t) (i >> 32);
            if (x == 0) {
                n += 32;
                x = (int64_t) i;
            }
            if (x >> 16 == 0) {
                n += 16;
                x <<= 16;
            }
            if (x >> 24 == 0) {
                n += 8;
                x <<= 8;
            }
            if (x >> 28 == 0) {
                n += 4;
                x <<= 4;
            }
            if (x >> 30 == 0) {
                n += 2;
                x <<= 2;
            }
            n -= (int) (x >> 31);
            return n;
        }
    }
}


namespace hazelcast {
    namespace util {
        SyncHttpClient::SyncHttpClient(const std::string &serverIp, const std::string &uriPath)
                : server(serverIp), uriPath(uriPath), socket(ioService), responseStream(&response) {
        }

        std::istream &SyncHttpClient::openConnection() {
            try {
                // Get a list of endpoints corresponding to the server name.
                boost::asio::ip::tcp::resolver resolver(ioService);
                boost::asio::ip::tcp::resolver::query query(server, "http");
                boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

                boost::asio::connect(socket, endpoint_iterator);

                socket.lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));

                // Form the request. We specify the "Connection: close" header so that the
                // server will close the socket after transmitting the response. This will
                // allow us to treat all data up until the EOF as the content.
                boost::asio::streambuf request;
                std::ostream request_stream(&request);
                request_stream << "GET " << uriPath << " HTTP/1.0\r\n";
                request_stream << "Host: " << server << "\r\n";
                request_stream << "Accept: */*\r\n";
                request_stream << "Connection: close\r\n\r\n";

                // Send the request.
                boost::asio::write(socket, request.data());

                // Read the response status line. The response streambuf will automatically
                // grow to accommodate the entire line. The growth may be limited by passing
                // a maximum size to the streambuf constructor.
                boost::asio::read_until(socket, response, "\r\n");

                // Check that response is OK.
                std::string httpVersion;
                responseStream >> httpVersion;
                unsigned int statusCode;
                responseStream >> statusCode;
                std::string statusMessage;
                std::getline(responseStream, statusMessage);
                if (!responseStream || httpVersion.substr(0, 5) != "HTTP/") {
                    throw client::exception::IOException("openConnection", "Invalid response");
                }
                if (statusCode != 200) {
                    std::stringstream out;
                    out << "Response returned with status: " << statusCode << " Status message:" << statusMessage;
                    throw client::exception::IOException("SyncHttpClient::openConnection", out.str());;
                }

                // Read the response headers, which are terminated by a blank line.
                boost::asio::read_until(socket, response, "\r\n\r\n");

                // Process the response headers.
                std::string header;
                while (std::getline(responseStream, header) && header != "\r");

                // Read until EOF
                boost::system::error_code error;
                size_t bytesRead;
                while ((bytesRead = boost::asio::read(socket, response.prepare(1024),
                                               boost::asio::transfer_at_least(1), error))) {
                    response.commit(bytesRead);
                }

                if (error != boost::asio::error::eof) {
                    throw boost::system::system_error(error);
                }

                return responseStream;
            } catch (boost::system::system_error &e) {
                std::ostringstream out;
                out << "Could not retrieve response from http://" << server << uriPath << " Error:" << e.what();
                throw client::exception::IOException("SyncHttpClient::openConnection", out.str());
            }
        }
    }
}


namespace hazelcast {
    namespace util {
        void IOUtil::closeResource(Closeable *closable, const char *closeReason) {
            if (closable != NULL) {
                try {
                    closable->close(closeReason);
                } catch (client::exception::IException &) {
                    // suppress
                }

            }
        }

        template<>
        bool IOUtil::to_value(const std::string &str) {
            return str == "true" || str == "1";
        }
    }
}




namespace hazelcast {
    namespace util {
        AtomicBoolean::AtomicBoolean() : std::atomic<bool>(false) {
        }

        AtomicBoolean::AtomicBoolean(bool i) : std::atomic<bool>(i) {
        }
    }
}




namespace hazelcast {
    namespace util {
        namespace concurrent {
            namespace locks {

                void LockSupport::parkNanos(int64_t nanos) {
                    if (nanos <= 0) {
                        return;
                    }

                    std::condition_variable conditionVariable;
                    std::mutex mtx;
                    std::unique_lock<std::mutex> lock(mtx);
                    conditionVariable.wait_for(lock, std::chrono::nanoseconds(nanos));
                }
            }
        }
    }
}

namespace hazelcast {
    namespace util {
        namespace concurrent {
            BackoffIdleStrategy::BackoffIdleStrategy(int64_t maxSpins, int64_t maxYields, int64_t minParkPeriodNs,
                                                     int64_t maxParkPeriodNs) {
                Preconditions::checkNotNegative(maxSpins, "maxSpins must be positive or zero");
                Preconditions::checkNotNegative(maxYields, "maxYields must be positive or zero");
                Preconditions::checkNotNegative(minParkPeriodNs, "minParkPeriodNs must be positive or zero");
                Preconditions::checkNotNegative(maxParkPeriodNs - minParkPeriodNs,
                                                "maxParkPeriodNs must be greater than or equal to minParkPeriodNs");
                this->yieldThreshold = maxSpins;
                this->parkThreshold = maxSpins + maxYields;
                this->minParkPeriodNs = minParkPeriodNs;
                this->maxParkPeriodNs = maxParkPeriodNs;
                this->maxShift = Int64Util::numberOfLeadingZeros(minParkPeriodNs) -
                                 Int64Util::numberOfLeadingZeros(maxParkPeriodNs);

            }

            bool BackoffIdleStrategy::idle(int64_t n) {
                if (n < yieldThreshold) {
                    return false;
                }
                if (n < parkThreshold) {
                    std::this_thread::yield();
                    return false;
                }
                int64_t time = parkTime(n);
                locks::LockSupport::parkNanos(time);
                return time == maxParkPeriodNs;
            }

            int64_t BackoffIdleStrategy::parkTime(int64_t n) const {
                const int64_t proposedShift = n - parkThreshold;
                const int64_t allowedShift = min<int64_t>(maxShift, proposedShift);
                return proposedShift > maxShift ? maxParkPeriodNs
                                                : proposedShift < maxShift ? minParkPeriodNs << allowedShift
                                                                           : min(minParkPeriodNs << allowedShift,
                                                                                 maxParkPeriodNs);
            }
        }
    }
}



namespace hazelcast {
    namespace util {
        namespace concurrent {
            CancellationException::CancellationException(const std::string &source, const std::string &message)
                    : IllegalStateException("CancellationException", client::protocol::CANCELLATION, source, message,
                                            "", true) {}
        }
    }
}




namespace hazelcast {
    namespace util {
        namespace concurrent {
            const NanoSeconds TimeUnit::NANOS;
            const MicroSeconds TimeUnit::MICROS;
            const MilliSeconds TimeUnit::MILLIS;
            const Seconds TimeUnit::SECS;
            const Minutes TimeUnit::MINS;
            const Hours TimeUnit::HRS;
            const Days TimeUnit::DS;

            const TimeUnit &TimeUnit::NANOSECONDS() {
                return TimeUnit::NANOS;
            }

            const TimeUnit &TimeUnit::MICROSECONDS() {
                return TimeUnit::MICROS;
            }

            const TimeUnit &TimeUnit::MILLISECONDS() {
                return TimeUnit::MILLIS;
            }

            const TimeUnit &TimeUnit::SECONDS() {
                return TimeUnit::SECS;
            }

            const TimeUnit &TimeUnit::MINUTES() {
                return TimeUnit::MINS;
            }

            const TimeUnit &TimeUnit::HOURS() {
                return TimeUnit::HRS;
            }

            const TimeUnit &TimeUnit::DAYS() {
                return TimeUnit::DS;
            }

            /**
             * Scale d by m, checking for overflow.
             * This has a short name to make above code more readable.
             */
            int64_t TimeUnit::x(int64_t d, int64_t m, int64_t over) {
                if (d > over) return INT64_MAX;
                if (d < -over) return INT64_MIN;
                return d * m;
            }

            int64_t NanoSeconds::toNanos(int64_t d) const { return d; }

            int64_t NanoSeconds::toMicros(int64_t d) const { return d / (C1 / C0); }

            int64_t NanoSeconds::toMillis(int64_t d) const { return d / (C2 / C0); }

            int64_t NanoSeconds::toSeconds(int64_t d) const { return d / (C3 / C0); }

            int64_t NanoSeconds::toMinutes(int64_t d) const { return d / (C4 / C0); }

            int64_t NanoSeconds::toHours(int64_t d) const { return d / (C5 / C0); }

            int64_t NanoSeconds::toDays(int64_t d) const { return d / (C6 / C0); }

            int64_t NanoSeconds::convert(int64_t d, const TimeUnit &u) const { return u.toNanos(d); }

            int64_t MicroSeconds::toNanos(int64_t d) const { return x(d, C1 / C0, MAX / (C1 / C0)); }

            int64_t MicroSeconds::toMicros(int64_t d) const { return d; }

            int64_t MicroSeconds::toMillis(int64_t d) const { return d / (C2 / C1); }

            int64_t MicroSeconds::toSeconds(int64_t d) const { return d / (C3 / C1); }

            int64_t MicroSeconds::toMinutes(int64_t d) const { return d / (C4 / C1); }

            int64_t MicroSeconds::toHours(int64_t d) const { return d / (C5 / C1); }

            int64_t MicroSeconds::toDays(int64_t d) const { return d / (C6 / C1); }

            int64_t MicroSeconds::convert(int64_t d, const TimeUnit &u) const { return u.toMicros(d); }

            int64_t MilliSeconds::toNanos(int64_t d) const { return x(d, C2 / C0, MAX / (C2 / C0)); }

            int64_t MilliSeconds::toMicros(int64_t d) const { return x(d, C2 / C1, MAX / (C2 / C1)); }

            int64_t MilliSeconds::toMillis(int64_t d) const { return d; }

            int64_t MilliSeconds::toSeconds(int64_t d) const { return d / (C3 / C2); }

            int64_t MilliSeconds::toMinutes(int64_t d) const { return d / (C4 / C2); }

            int64_t MilliSeconds::toHours(int64_t d) const { return d / (C5 / C2); }

            int64_t MilliSeconds::toDays(int64_t d) const { return d / (C6 / C2); }

            int64_t MilliSeconds::convert(int64_t d, const TimeUnit &u) const { return u.toMillis(d); }

            int64_t Seconds::toNanos(int64_t d) const { return x(d, C3 / C0, MAX / (C3 / C0)); }

            int64_t Seconds::toMicros(int64_t d) const { return x(d, C3 / C1, MAX / (C3 / C1)); }

            int64_t Seconds::toMillis(int64_t d) const { return x(d, C3 / C2, MAX / (C3 / C2)); }

            int64_t Seconds::toSeconds(int64_t d) const { return d; }

            int64_t Seconds::toMinutes(int64_t d) const { return d / (C4 / C3); }

            int64_t Seconds::toHours(int64_t d) const { return d / (C5 / C3); }

            int64_t Seconds::toDays(int64_t d) const { return d / (C6 / C3); }

            int64_t Seconds::convert(int64_t d, const TimeUnit &u) const { return u.toSeconds(d); }

            int64_t Minutes::toNanos(int64_t d) const { return x(d, C4 / C0, MAX / (C4 / C0)); }

            int64_t Minutes::toMicros(int64_t d) const { return x(d, C4 / C1, MAX / (C4 / C1)); }

            int64_t Minutes::toMillis(int64_t d) const { return x(d, C4 / C2, MAX / (C4 / C2)); }

            int64_t Minutes::toSeconds(int64_t d) const { return x(d, C4 / C3, MAX / (C4 / C3)); }

            int64_t Minutes::toMinutes(int64_t d) const { return d; }

            int64_t Minutes::toHours(int64_t d) const { return d / (C5 / C4); }

            int64_t Minutes::toDays(int64_t d) const { return d / (C6 / C4); }

            int64_t Minutes::convert(int64_t d, const TimeUnit &u) const { return u.toMinutes(d); }

            int64_t Hours::toNanos(int64_t d) const { return x(d, C5 / C0, MAX / (C5 / C0)); }

            int64_t Hours::toMicros(int64_t d) const { return x(d, C5 / C1, MAX / (C5 / C1)); }

            int64_t Hours::toMillis(int64_t d) const { return x(d, C5 / C2, MAX / (C5 / C2)); }

            int64_t Hours::toSeconds(int64_t d) const { return x(d, C5 / C3, MAX / (C5 / C3)); }

            int64_t Hours::toMinutes(int64_t d) const { return x(d, C5 / C4, MAX / (C5 / C4)); }

            int64_t Hours::toHours(int64_t d) const { return d; }

            int64_t Hours::toDays(int64_t d) const { return d / (C6 / C5); }

            int64_t Hours::convert(int64_t d, const TimeUnit &u) const { return u.toHours(d); }

            int64_t Days::toNanos(int64_t d) const { return x(d, C6 / C0, MAX / (C6 / C0)); }

            int64_t Days::toMicros(int64_t d) const { return x(d, C6 / C1, MAX / (C6 / C1)); }

            int64_t Days::toMillis(int64_t d) const { return x(d, C6 / C2, MAX / (C6 / C2)); }

            int64_t Days::toSeconds(int64_t d) const { return x(d, C6 / C3, MAX / (C6 / C3)); }

            int64_t Days::toMinutes(int64_t d) const { return x(d, C6 / C4, MAX / (C6 / C4)); }

            int64_t Days::toHours(int64_t d) const { return x(d, C6 / C5, MAX / (C6 / C5)); }

            int64_t Days::toDays(int64_t d) const { return d; }

            int64_t Days::convert(int64_t d, const TimeUnit &u) const { return u.toDays(d); }
        }
    }
}

//
// Created by ihsan demir on 9 Dec 2016.


namespace hazelcast {
    namespace util {
        const std::string &Preconditions::checkHasText(const std::string &argument,
                                                       const std::string &errorMessage) {
            if (argument.empty()) {
                throw client::exception::IllegalArgumentException("", errorMessage);
            }

            return argument;
        }

        void Preconditions::checkSSL(const std::string &sourceMethod) {
#ifndef HZ_BUILD_WITH_SSL
            throw client::exception::InvalidConfigurationException(sourceMethod, "You should compile with "
                    "HZ_BUILD_WITH_SSL flag. You should also have the openssl installed on your machine and you need "
                    "to link with the openssl library.");
#endif
        }

        void Preconditions::checkTrue(bool expression, const std::string &errorMessage) {
            if (!expression) {
                throw client::exception::IllegalArgumentException(errorMessage);
            }
        }
    }
}




namespace hazelcast {
    namespace util {
        std::string UuidUtil::newUnsecureUuidString() {
            return newUnsecureUUID().toString();
        }

        UUID UuidUtil::newUnsecureUUID() {
            byte data[16];
            // TODO: Use a better random bytes generator
            for (int j = 0; j < 16; ++j) {
                data[j] = rand() % 16;
            }

            // clear version
            data[6] &= 0x0f;
            // set to version 4
            data[6] |= 0x40;
            // clear variant
            data[8] &= 0x3f;
            // set to IETF variant
            data[8] |= 0x80;

            int64_t mostSigBits = 0;
            int64_t leastSigBits = 0;
            for (int i = 0; i < 8; i++) {
                mostSigBits = (mostSigBits << 8) | (data[i] & 0xff);
            }
            for (int i = 8; i < 16; i++) {
                leastSigBits = (leastSigBits << 8) | (data[i] & 0xff);
            }
            return UUID(mostSigBits, leastSigBits);
        }
    }
}


namespace hazelcast {
    namespace util {
        AtomicInt::AtomicInt() : atomic<int>(0) {
        }

        AtomicInt::AtomicInt(const int &value) : atomic<int>(value) {
        }
    }
}




namespace hazelcast {
    namespace util {
        const int AddressHelper::MAX_PORT_TRIES = 3;
        const int AddressHelper::INITIAL_FIRST_PORT = 5701;

        std::vector<client::Address> AddressHelper::getSocketAddresses(const std::string &address, ILogger &logger) {
            const AddressHolder addressHolder = AddressUtil::getAddressHolder(address, -1);
            const std::string scopedAddress = !addressHolder.getScopeId().empty()
                                              ? addressHolder.getAddress() + '%' + addressHolder.getScopeId()
                                              : addressHolder.getAddress();

            int port = addressHolder.getPort();
            int maxPortTryCount = 1;
            if (port == -1) {
                maxPortTryCount = MAX_PORT_TRIES;
            }
            return getPossibleSocketAddresses(port, scopedAddress, maxPortTryCount, logger);
        }

        std::vector<client::Address>
        AddressHelper::getPossibleSocketAddresses(int port, const std::string &scopedAddress, int portTryCount,
                                                  ILogger &logger) {
            std::unique_ptr<boost::asio::ip::address> inetAddress;
            try {
                inetAddress.reset(new boost::asio::ip::address(AddressUtil::getByName(scopedAddress)));
            } catch (client::exception::UnknownHostException &ignored) {
                logger.finest("Address ", scopedAddress, " ip number is not available", ignored.what());
            }

            int possiblePort = port;
            if (possiblePort == -1) {
                possiblePort = INITIAL_FIRST_PORT;
            }
            std::vector<client::Address> addresses;

            if (!inetAddress.get()) {
                for (int i = 0; i < portTryCount; i++) {
                    try {
                        addresses.push_back(client::Address(scopedAddress, possiblePort + i));
                    } catch (client::exception::UnknownHostException &ignored) {
                        std::ostringstream out;
                        out << "Address [" << scopedAddress << "] ip number is not available." << ignored.what();
                        logger.finest(out.str());
                    }
                }
            } else if (inetAddress->is_v4() || inetAddress->is_v6()) {
                for (int i = 0; i < portTryCount; i++) {
                    if (inetAddress->is_v4()) {
                        addresses.push_back(client::Address(scopedAddress, possiblePort + i));
                    } else {
                        addresses.push_back(
                                client::Address(scopedAddress, possiblePort + i, inetAddress->to_v6().scope_id()));
                    }
                }
            }
            // TODO: Add ip v6 addresses using interfaces as done in Java client.

            return addresses;
        }

        AddressHolder::AddressHolder(const std::string &address, const std::string &scopeId, int port) : address(
                address), scopeId(scopeId), port(port) {}

        std::ostream &operator<<(std::ostream &os, const AddressHolder &holder) {
            os << "AddressHolder [" << holder.address + "]:" << holder.port;
            return os;
        }

        const std::string &AddressHolder::getAddress() const {
            return address;
        }

        const std::string &AddressHolder::getScopeId() const {
            return scopeId;
        }

        int AddressHolder::getPort() const {
            return port;
        }
    }
}


namespace hazelcast {
    namespace util {
        util::AtomicInt RuntimeAvailableProcessors::currentAvailableProcessors(
                RuntimeAvailableProcessors::getNumberOfProcessors());

        int RuntimeAvailableProcessors::getNumberOfProcessors() {
#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            return sysinfo.dwNumberOfProcessors;
#else
            return sysconf(_SC_NPROCESSORS_ONLN);
#endif
        }

        int RuntimeAvailableProcessors::get() {
            return currentAvailableProcessors;
        }

        void RuntimeAvailableProcessors::override(int availableProcessors) {
            RuntimeAvailableProcessors::currentAvailableProcessors.store(availableProcessors);
        }

        void RuntimeAvailableProcessors::resetOverride() {
            currentAvailableProcessors.store(getNumberOfProcessors());
        }
    }
}


//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.


//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline


#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

#define    FORCE_INLINE inline __attribute__((always_inline))

FORCE_INLINE  uint32_t rotl32(uint32_t x, int8_t r) {
    return (x << r) | (x >> (32 - r));
}

FORCE_INLINE  uint64_t rotl64(uint64_t x, int8_t r) {
    return (x << r) | (x >> (64 - r));
}

#define    ROTL32(x, y)    rotl32(x,y)
#define ROTL64(x, y)    rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

namespace hazelcast {
    namespace util {
        FORCE_INLINE uint32_t getblock32(const uint32_t *p, int i) {
            return *(p + i);
        }

        FORCE_INLINE uint64_t getblock64(const uint64_t *p, int i) {
            return *(p + i);
        }

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

        FORCE_INLINE uint32_t fmix32(uint32_t h) {
            h ^= h >> 16;
            h *= 0x85ebca6b;
            h ^= h >> 13;
            h *= 0xc2b2ae35;
            h ^= h >> 16;

            return h;
        }

//----------

        FORCE_INLINE uint64_t fmix64(uint64_t k) {
            k ^= k >> 33;
            k *= BIG_CONSTANT(0xff51afd7ed558ccd);
            k ^= k >> 33;
            k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
            k ^= k >> 33;

            return k;
        }

//-----------------------------------------------------------------------------
        int MurmurHash3_x86_32(const void *key, int len) {
            uint32_t DEFAULT_MURMUR_SEED = 0x01000193;
            uint32_t hash = 0;
            MurmurHash3_x86_32(key, len, DEFAULT_MURMUR_SEED, (void *) &hash);
            return hash;
        }


        void MurmurHash3_x86_32(const void *key, int len, uint32_t seed, void *out) {
            const uint8_t *data = (const uint8_t *) key;
            const int nblocks = len / 4;

            uint32_t h1 = seed;

            const uint32_t c1 = 0xcc9e2d51;
            const uint32_t c2 = 0x1b873593;

            //----------
            // body

            const uint32_t *blocks = (const uint32_t *) (data + nblocks * 4);

            for (int i = -nblocks; i; i++) {
                uint32_t k1 = getblock32(blocks, i);

                k1 *= c1;
                k1 = ROTL32(k1, 15);
                k1 *= c2;

                h1 ^= k1;
                h1 = ROTL32(h1, 13);
                h1 = h1 * 5 + 0xe6546b64;
            }

            //----------
            // tail

            const uint8_t *tail = (const uint8_t *) (data + nblocks * 4);

            uint32_t k1 = 0;

            switch (len & 3) {
                case 3:
                    k1 ^= tail[2] << 16;
                case 2:
                    k1 ^= tail[1] << 8;
                case 1:
                    k1 ^= tail[0];
                    k1 *= c1;
                    k1 = ROTL32(k1, 15);
                    k1 *= c2;
                    h1 ^= k1;
            };

            //----------
            // finalization

            h1 ^= len;

            h1 = fmix32(h1);

            *(uint32_t *) out = h1;
        }

//-----------------------------------------------------------------------------
    }
}
//-----------------------------------------------------------------------------


//  Copyright (c) 2015 ihsan demir. All rights reserved.
//



namespace hazelcast {
    namespace util {
        const std::shared_ptr<ExceptionUtil::RuntimeExceptionFactory> ExceptionUtil::hazelcastExceptionFactory(
                new HazelcastExceptionFactory());

        void ExceptionUtil::rethrow(std::exception_ptr e) {
            return rethrow(e, HAZELCAST_EXCEPTION_FACTORY());
        }

        void ExceptionUtil::rethrow(std::exception_ptr e,
                                    const std::shared_ptr<ExceptionUtil::RuntimeExceptionFactory> &runtimeExceptionFactory) {
            try {
                std::rethrow_exception(e);
            } catch (client::exception::IException &ie) {
                if (ie.isRuntimeException()) {
                    std::rethrow_exception(e);
                }

                int32_t errorCode = ie.getErrorCode();
                if (errorCode == client::protocol::EXECUTION) {
                    try {
                        std::rethrow_if_nested(std::current_exception());
                    } catch (...) {
                        rethrow(std::current_exception(), runtimeExceptionFactory);
                    }
                }

                runtimeExceptionFactory->rethrow(e, "");
            }
        }

        const std::shared_ptr<ExceptionUtil::RuntimeExceptionFactory> &ExceptionUtil::HAZELCAST_EXCEPTION_FACTORY() {
            return hazelcastExceptionFactory;
        }

        ExceptionUtil::RuntimeExceptionFactory::~RuntimeExceptionFactory() {
        }

        void ExceptionUtil::HazelcastExceptionFactory::rethrow(
                std::exception_ptr throwable, const std::string &message) {
            try {
                std::rethrow_exception(throwable);
            } catch (...) {
                std::throw_with_nested(boost::enable_current_exception(
                        client::exception::HazelcastException("HazelcastExceptionFactory::create", message)));
            }
        }
    }
}

namespace hazelcast {
    namespace util {

        ByteBuffer::ByteBuffer(char *buffer, size_t capacity)
                : pos(0), lim(capacity), capacity(capacity), buffer(buffer) {

        }

        ByteBuffer &ByteBuffer::flip() {
            lim = pos;
            pos = 0;
            return *this;
        }


        ByteBuffer &ByteBuffer::compact() {
            memcpy(buffer, ix(), (size_t) remaining());
            pos = remaining();
            lim = capacity;
            return *this;
        }

        ByteBuffer &ByteBuffer::clear() {
            pos = 0;
            lim = capacity;
            return *this;
        }

        size_t ByteBuffer::remaining() const {
            return lim - pos;
        }

        bool ByteBuffer::hasRemaining() const {
            return pos < lim;
        }

        size_t ByteBuffer::position() const {
            return pos;
        }

        int ByteBuffer::readInt() {
            char a = readByte();
            char b = readByte();
            char c = readByte();
            char d = readByte();
            return (0xff000000 & (a << 24)) |
                   (0x00ff0000 & (b << 16)) |
                   (0x0000ff00 & (c << 8)) |
                   (0x000000ff & d);
        }

        void ByteBuffer::writeInt(int v) {
            writeByte(char(v >> 24));
            writeByte(char(v >> 16));
            writeByte(char(v >> 8));
            writeByte(char(v));
        }


        short ByteBuffer::readShort() {
            byte a = readByte();
            byte b = readByte();
            return (short) ((0xff00 & (a << 8)) |
                            (0x00ff & b));
        }

        void ByteBuffer::writeShort(short v) {
            writeByte(char(v >> 8));
            writeByte(char(v));
        }

        byte ByteBuffer::readByte() {
            byte b = (byte) buffer[pos];
            safeIncrementPosition(1);
            return b;
        }

        void ByteBuffer::writeByte(char c) {
            buffer[pos] = c;
            safeIncrementPosition(1);
        }

        void *ByteBuffer::ix() const {
            return (void *) (buffer + pos);
        }

        void ByteBuffer::safeIncrementPosition(size_t t) {
            assert(pos + t <= capacity);
            pos += t;
        }

        size_t ByteBuffer::readBytes(byte *target, size_t len) {
            size_t numBytesToCopy = util::min<size_t>(lim - pos, len);
            memcpy(target, ix(), numBytesToCopy);
            pos += numBytesToCopy;
            return numBytesToCopy;
        }

        hz_thread_pool::hz_thread_pool(size_t numThreads) : pool_(new boost::asio::thread_pool(numThreads)) {}

        void hz_thread_pool::shutdown_gracefully() {
            pool_->join();

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            // needed due to bug https://github.com/chriskohlhoff/asio/issues/431
            boost::asio::use_service<boost::asio::detail::win_iocp_io_context>(*pool_).stop();
#endif
            pool_.reset();
        }

        boost::asio::thread_pool::executor_type hz_thread_pool::get_executor() const {
            return pool_->get_executor();
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
