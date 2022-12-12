/*
 * Copyright (c) 2008-2022, Hazelcast, Inc. All Rights Reserved.
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

#include <algorithm>
#include <cassert>
#include <chrono>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <limits>
#include <mutex>
#include <sstream>
#include <thread>

#include <boost/concept_check.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/algorithm/string/classification.hpp>

#ifdef HZ_BUILD_WITH_SSL
#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/basic_resolver.hpp>
#include <boost/asio/ssl/rfc2818_verification.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/system_error.hpp>
#endif // HZ_BUILD_WITH_SSL

#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/AddressUtil.h"
#include "hazelcast/util/HashUtil.h"
#include "hazelcast/util/Util.h"
#include "hazelcast/util/Preconditions.h"
#include "hazelcast/util/Clearable.h"
#include "hazelcast/util/hz_thread_pool.h"
#include "hazelcast/util/Destroyable.h"
#include "hazelcast/util/Closeable.h"
#include "hazelcast/util/SyncHttpClient.h"
#include "hazelcast/util/concurrent/locks/LockSupport.h"
#include "hazelcast/util/concurrent/BackoffIdleStrategy.h"
#include "hazelcast/util/AddressHelper.h"
#include "hazelcast/util/MurmurHash3.h"
#include "hazelcast/client/exception/protocol_exceptions.h"
#include "hazelcast/util/ByteBuffer.h"
#include "hazelcast/util/exception_util.h"
#include "hazelcast/logger.h"

#ifdef HZ_BUILD_WITH_SSL
#include <hazelcast/util/SyncHttpsClient.h>
#endif // HZ_BUILD_WITH_SSL

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4996) // for strerror
#endif

namespace hazelcast {
namespace util {

AddressHolder
AddressUtil::get_address_holder(const std::string& address, int default_port)
{
    int indexBracketStart = static_cast<int>(address.find('['));
    int indexBracketEnd =
      static_cast<int>(address.find(']', indexBracketStart));
    int indexColon = static_cast<int>(address.find(':'));
    int lastIndexColon = static_cast<int>(address.rfind(':'));
    std::string host;
    int port = default_port;
    std::string scopeId;
    if (indexColon > -1 && lastIndexColon > indexColon) {
        // IPv6
        if (indexBracketStart == 0 && indexBracketEnd > indexBracketStart) {
            host = address.substr(indexBracketStart + 1,
                                  indexBracketEnd - (indexBracketStart + 1));
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

AddressHolder
AddressUtil::get_address_holder(const std::string& address)
{
    return get_address_holder(address, -1);
}

boost::asio::ip::address
AddressUtil::get_by_name(const std::string& host)
{
    return get_by_name(host, "");
}

boost::asio::ip::address
AddressUtil::get_by_name(const std::string& host, const std::string& service)
{
    try {
        boost::asio::io_service ioService;
        boost::asio::ip::tcp::resolver res(ioService);
        boost::asio::ip::tcp::resolver::query query(host, service);
        boost::asio::ip::basic_resolver<boost::asio::ip::tcp>::iterator
          iterator = res.resolve(query);
        return iterator->endpoint().address();
    } catch (boost::system::system_error& e) {
        std::ostringstream out;
        out << "Address " << host << " ip number is not available. "
            << e.what();
        throw client::exception::unknown_host("AddressUtil::getByName",
                                              out.str());
    }
}

} // namespace util
} // namespace hazelcast

#ifdef HZ_BUILD_WITH_SSL
namespace hazelcast {
namespace util {
SyncHttpsClient::SyncHttpsClient(const std::string& server_ip,
                                 const std::string& uri_path,
                                 std::chrono::steady_clock::duration timeout,
                                 const std::string& secret_removal)
  : server_(server_ip)
  , uri_path_(uri_path)
  , timeout_(timeout)
  , secret_removal_(secret_removal)
  , resolver_(io_service_)
  , ssl_context_(boost::asio::ssl::context::tlsv12)
  , response_stream_(&response_)
{
    util::Preconditions::check_ssl("SyncHttpsClient::SyncHttpsClient");

    ssl_context_.set_default_verify_paths();
    ssl_context_.set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2 |
                             boost::asio::ssl::context::single_dh_use);

    ssl_context_.set_verify_mode(boost::asio::ssl::verify_peer);
    ssl_context_.set_verify_callback(
      boost::asio::ssl::rfc2818_verification(server_));
    socket_ =
      std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>>(
        new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(
          io_service_, ssl_context_));
}

std::istream&
SyncHttpsClient::connect_and_get_response()
{
    util::Preconditions::check_ssl("SyncHttpsClient::openConnection");

    std::chrono::steady_clock::time_point end_time =
      std::chrono::steady_clock::now() + timeout_;
    try {
        boost::system::error_code error;
        boost::asio::ip::tcp::resolver::results_type addresses;
        resolver_.async_resolve(
          server_,
          "https",
          [&](const boost::system::error_code& ec,
              boost::asio::ip::tcp::resolver::results_type results) {
              error = ec;
              addresses = results;
          });

        run(error, end_time - std::chrono::steady_clock::now());

        boost::asio::async_connect(
          socket_->lowest_layer(),
          addresses,
          [&](const boost::system::error_code& ec,
              const boost::asio::ip::tcp::endpoint& endpoint) { error = ec; });

        run(error, end_time - std::chrono::steady_clock::now());

        socket_->lowest_layer().set_option(
          boost::asio::ip::tcp::no_delay(true));

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(socket_->native_handle(),
                                      server_.c_str())) {
            boost::system::error_code ec{
                static_cast<int>(::ERR_get_error()),
                boost::asio::error::get_ssl_category()
            };
            throw std::system_error{ ec };
        }

        socket_->async_handshake(
          boost::asio::ssl::stream_base::client,
          [&](const boost::system::error_code& ec) { error = ec; });
        run(error, end_time - std::chrono::steady_clock::now());

        socket_->lowest_layer().set_option(
          boost::asio::ip::tcp::no_delay(true));

        // Form the request. We specify the "Connection: close" header so that
        // the server will close the socket after transmitting the response.
        // This will allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << uri_path_ << " HTTP/1.0\r\n";
        request_stream << "Host: " << server_ << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        boost::asio::async_write(*socket_,
                                 request.data(),
                                 [&](const boost::system::error_code& ec,
                                     std::size_t) { error = ec; });
        run(error, timeout_);

        // Read the response status line. The response streambuf will
        // automatically grow to accommodate the entire line. The growth may be
        // limited by passing a maximum size to the streambuf constructor. Read
        // until EOF
        end_time = std::chrono::steady_clock::now() + timeout_;
        while (!error) {
            boost::asio::async_read(
              *socket_,
              response_.prepare(1024),
              boost::asio::transfer_at_least(1),
              [&](const boost::system::error_code& ec, std::size_t bytes_read) {
                  error = ec;
                  if (error) {
                      return;
                  }
                  response_.commit(bytes_read);
              });

            run(error, end_time - std::chrono::steady_clock::now());
        }

        // Check that response is OK.
        std::string httpVersion;
        response_stream_ >> httpVersion;
        unsigned int statusCode;
        response_stream_ >> statusCode;
        std::string statusMessage;
        std::getline(response_stream_, statusMessage);
        if (!response_stream_ || httpVersion.substr(0, 5) != "HTTP/") {
            throw client::exception::io("openConnection", "Invalid response");
        }
        if (statusCode != 200) {
            std::stringstream out;
            out << "Response returned with status: " << statusCode
                << " Status message:" << statusMessage;
            throw client::exception::io("SyncHttpsClient::openConnection",
                                        out.str());
            ;
        }

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream_, header) && header != "\r")
            ;

    } catch (std::exception& e) {
        close();
        auto message =
          (boost::format(
             "Could not retrieve response from https://%1%%2%. Error:%3%") %
           server_ % uri_path_ % e.what())
            .str();

        if (!secret_removal_.empty())
            boost::replace_all(message, secret_removal_, "<SECRET_REMOVAL>");

        throw client::exception::io("SyncHttpsClient::openConnection",
                                    move(message));
    }
    return response_stream_;
}

void
SyncHttpsClient::run(boost::system::error_code& error,
                     std::chrono::steady_clock::duration timeout)
{
    // Restart the io_context, as it may have been left in the "stopped" state
    // by a previous operation.
    io_service_.restart();

    // Block until the asynchronous operation has completed, or timed out. If
    // the pending asynchronous operation is a composed operation, the deadline
    // applies to the entire operation, rather than individual operations on
    // the socket.
    io_service_.run_for(timeout);

    // If the asynchronous operation completed successfully then the io_context
    // would have been stopped due to running out of work. If it was not
    // stopped, then the io_context::run_for call must have timed out.
    if (!io_service_.stopped()) {
        // Close the socket to cancel the outstanding asynchronous operation.
        socket_->lowest_layer().close();

        // Run the io_context again until the operation completes.
        io_service_.run();
    }

    if (error && (error.value() != boost::asio::error::eof &&
                  error.value() != boost::asio::ssl::error::stream_truncated)) {
        throw std::system_error(error);
    }
}

void
SyncHttpsClient::close()
{
    boost::system::error_code ignored;
    socket_->lowest_layer().close(ignored);
}
} // namespace util
} // namespace hazelcast
#endif // HZ_BUILD_WITH_SSL

namespace hazelcast {
namespace util {
Clearable::~Clearable() = default;
}
} // namespace hazelcast

namespace hazelcast {
namespace util {
int
HashUtil::hash_to_index(int hash, int length)
{
    Preconditions::check_positive(length, "mod must be larger than 0");

    if (hash == INT_MIN) {
        hash = 0;
    } else {
        hash = std::abs(hash);
    }

    return hash % length;
}
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {
Destroyable::~Destroyable() = default;
}
} // namespace hazelcast

namespace hazelcast {
namespace util {
Closeable::~Closeable() = default;
}
} // namespace hazelcast

namespace hazelcast {
namespace util {
int64_t
get_current_thread_id()
{
    return std::hash<std::thread::id>{}(std::this_thread::get_id());
}

void
sleep(int seconds)
{
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}

int
localtime(const time_t* clock, struct tm* result)
{
    int returnCode = -1;
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    returnCode = localtime_s(result, clock);
#else
    if (NULL != localtime_r(clock, result)) {
        returnCode = 0;
    }
#endif

    return returnCode;
}

int
hz_snprintf(char* str, size_t len, const char* format, ...)
{
    va_list args;
    va_start(args, format);

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
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

void
git_date_to_hazelcast_log_date(std::string& date)
{
    // convert the date string from "2016-04-20" to 20160420
    date.erase(std::remove(date.begin(), date.end(), '"'), date.end());
    if (date != "NOT_FOUND") {
        date.erase(std::remove(date.begin(), date.end(), '-'), date.end());
    }
}

int64_t
current_time_millis()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

int64_t
current_time_nanos()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

int32_t
get_available_core_count()
{
    return (int32_t)std::thread::hardware_concurrency();
}

std::string
StringUtil::time_to_string(std::chrono::steady_clock::time_point t)
{
    using namespace std::chrono;

    if (!t.time_since_epoch().count()) {
        return std::string("never");
    }

    auto system_time =
      system_clock::now() +
      duration_cast<system_clock::duration>(t - steady_clock::now());
    auto msecs =
      duration_cast<milliseconds>(system_time.time_since_epoch()).count() %
      1000;

    auto brokenTime = system_clock::to_time_t(system_time);
    struct tm localBrokenTime;
    int result = util::localtime(&brokenTime, &localBrokenTime);
    assert(!result);
    boost::ignore_unused_variable_warning(result);

    std::ostringstream oss;
    char time_buffer[80];
    std::strftime(
      time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", &localBrokenTime);
    oss << time_buffer;
    oss << '.' << std::setfill('0') << std::setw(3) << msecs;

    return oss.str();
}

int
Int64Util::number_of_leading_zeros(int64_t i)
{
    // HD, Figure 5-6
    if (i == 0)
        return 64;
    int n = 1;
    int64_t x = (int64_t)(i >> 32);
    if (x == 0) {
        n += 32;
        x = (int64_t)i;
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
    n -= (int)(x >> 31);
    return n;
}
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {
SyncHttpClient::SyncHttpClient(const std::string& server_ip,
                               const std::string& uri_path)
  : server_(server_ip)
  , uri_path_(uri_path)
  , socket_(io_service_)
  , response_stream_(&response_)
{}

std::istream&
SyncHttpClient::open_connection()
{
    try {
        // Get a list of endpoints corresponding to the server name.
        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(server_, "http");
        boost::asio::ip::tcp::resolver::iterator endpoint_iterator =
          resolver.resolve(query);

        boost::asio::connect(socket_, endpoint_iterator);

        socket_.lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));

        // Form the request. We specify the "Connection: close" header so that
        // the server will close the socket after transmitting the response.
        // This will allow us to treat all data up until the EOF as the content.
        boost::asio::streambuf request;
        std::ostream request_stream(&request);
        request_stream << "GET " << uri_path_ << " HTTP/1.0\r\n";
        request_stream << "Host: " << server_ << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        boost::asio::write(socket_, request.data());

        // Read the response status line. The response streambuf will
        // automatically grow to accommodate the entire line. The growth may be
        // limited by passing a maximum size to the streambuf constructor.
        boost::asio::read_until(socket_, response_, "\r\n");

        // Check that response is OK.
        std::string httpVersion;
        response_stream_ >> httpVersion;
        unsigned int statusCode;
        response_stream_ >> statusCode;
        std::string statusMessage;
        std::getline(response_stream_, statusMessage);
        if (!response_stream_ || httpVersion.substr(0, 5) != "HTTP/") {
            throw client::exception::io("openConnection", "Invalid response");
        }
        if (statusCode != 200) {
            std::stringstream out;
            out << "Response returned with status: " << statusCode
                << " Status message:" << statusMessage;
            throw client::exception::io("SyncHttpClient::openConnection",
                                        out.str());
            ;
        }

        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket_, response_, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream_, header) && header != "\r")
            ;

        // Read until EOF
        boost::system::error_code error;
        size_t bytesRead;
        while ((bytesRead = boost::asio::read(socket_,
                                              response_.prepare(1024),
                                              boost::asio::transfer_at_least(1),
                                              error))) {
            response_.commit(bytesRead);
        }

        if (error != boost::asio::error::eof) {
            throw boost::system::system_error(error);
        }

        return response_stream_;
    } catch (boost::system::system_error& e) {
        std::ostringstream out;
        out << "Could not retrieve response from http://" << server_
            << uri_path_ << " Error:" << e.what();
        throw client::exception::io("SyncHttpClient::openConnection",
                                    out.str());
    }
}
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {
void
IOUtil::close_resource(Closeable* closable, const char* close_reason)
{
    if (closable != NULL) {
        try {
            closable->close(close_reason);
        } catch (client::exception::iexception&) {
            // suppress
        }
    }
}

template<>
bool
IOUtil::to_value(const std::string& str)
{
    return str == "true" || str == "1";
}
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {
namespace concurrent {
namespace locks {

void
LockSupport::park_nanos(int64_t nanos)
{
    if (nanos <= 0) {
        return;
    }

    std::condition_variable conditionVariable;
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    conditionVariable.wait_for(lock, std::chrono::nanoseconds(nanos));
}
} // namespace locks
} // namespace concurrent
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {
namespace concurrent {
BackoffIdleStrategy::BackoffIdleStrategy(int64_t max_spins,
                                         int64_t max_yields,
                                         int64_t min_park_period_ns,
                                         int64_t max_park_period_ns)
{
    Preconditions::check_not_negative(max_spins,
                                      "maxSpins must be positive or zero");
    Preconditions::check_not_negative(max_yields,
                                      "maxYields must be positive or zero");
    Preconditions::check_not_negative(
      min_park_period_ns, "minParkPeriodNs must be positive or zero");
    Preconditions::check_not_negative(
      max_park_period_ns - min_park_period_ns,
      "maxParkPeriodNs must be greater than or equal to minParkPeriodNs");
    this->yield_threshold_ = max_spins;
    this->park_threshold_ = max_spins + max_yields;
    this->min_park_period_ns_ = min_park_period_ns;
    this->max_park_period_ns_ = max_park_period_ns;
    this->max_shift_ = Int64Util::number_of_leading_zeros(min_park_period_ns) -
                       Int64Util::number_of_leading_zeros(max_park_period_ns);
}

bool
BackoffIdleStrategy::idle(int64_t n)
{
    if (n < yield_threshold_) {
        return false;
    }
    if (n < park_threshold_) {
        std::this_thread::yield();
        return false;
    }
    int64_t time = park_time(n);
    locks::LockSupport::park_nanos(time);
    return time == max_park_period_ns_;
}

int64_t
BackoffIdleStrategy::park_time(int64_t n) const
{
    const int64_t proposedShift = n - park_threshold_;
    const int64_t allowedShift = min<int64_t>(max_shift_, proposedShift);
    return proposedShift > max_shift_ ? max_park_period_ns_
           : proposedShift < max_shift_
             ? min_park_period_ns_ << allowedShift
             : (min)(min_park_period_ns_ << allowedShift, max_park_period_ns_);
}
} // namespace concurrent
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {
const std::string&
Preconditions::check_has_text(const std::string& argument,
                              const std::string& error_message)
{
    if (argument.empty()) {
        throw client::exception::illegal_argument("", error_message);
    }

    return argument;
}

void
Preconditions::check_ssl(const std::string& source_method)
{
#ifndef HZ_BUILD_WITH_SSL
    throw client::exception::invalid_configuration(
      source_method,
      "You should compile with "
      "HZ_BUILD_WITH_SSL flag. You should also have the openssl installed on "
      "your machine and you need "
      "to link with the openssl library.");
#endif
}

void
Preconditions::check_true(bool expression, const std::string& error_message)
{
    if (!expression) {
        throw client::exception::illegal_argument(error_message);
    }
}
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {
const int AddressHelper::MAX_PORT_TRIES = 3;
const int AddressHelper::INITIAL_FIRST_PORT = 5701;

std::vector<client::address>
AddressHelper::get_socket_addresses(const std::string& address, logger& lg)
{
    const AddressHolder addressHolder =
      AddressUtil::get_address_holder(address, -1);
    std::string scopedAddress = get_scoped_hostname(addressHolder);

    int port = addressHolder.get_port();
    int maxPortTryCount = 1;
    if (port == -1) {
        maxPortTryCount = MAX_PORT_TRIES;
    }
    return get_possible_socket_addresses(
      port, scopedAddress, maxPortTryCount, lg);
}

std::string
AddressHelper::get_scoped_hostname(const AddressHolder& addressHolder)
{
    const std::string scopedAddress =
      !addressHolder.get_scope_id().empty()
        ? addressHolder.get_address() + '%' + addressHolder.get_scope_id()
        : addressHolder.get_address();
    return scopedAddress;
}

std::vector<client::address>
AddressHelper::get_possible_socket_addresses(int port,
                                             const std::string& scoped_address,
                                             int port_try_count,
                                             logger& lg)
{
    std::unique_ptr<boost::asio::ip::address> inetAddress;
    try {
        inetAddress.reset(new boost::asio::ip::address(
          AddressUtil::get_by_name(scoped_address)));
    } catch (client::exception::unknown_host& ignored) {
        HZ_LOG(lg,
               finest,
               boost::str(
                 boost::format("Address %1% ip number is not available %2%") %
                 scoped_address % ignored.what()));
    }

    int possiblePort = port;
    if (possiblePort == -1) {
        possiblePort = INITIAL_FIRST_PORT;
    }
    std::vector<client::address> addresses;

    if (!inetAddress.get()) {
        for (int i = 0; i < port_try_count; i++) {
            try {
                addresses.push_back(
                  client::address(scoped_address, possiblePort + i));
            } catch (client::exception::unknown_host& ignored) {
                HZ_LOG(lg,
                       finest,
                       boost::str(
                         boost::format(
                           "Address [%1%] ip number is not available. %2%") %
                         scoped_address % ignored.what()));
            }
        }
    } else if (inetAddress->is_v4() || inetAddress->is_v6()) {
        for (int i = 0; i < port_try_count; i++) {
            if (inetAddress->is_v4()) {
                addresses.push_back(
                  client::address(scoped_address, possiblePort + i));
            } else {
                addresses.push_back(
                  client::address(scoped_address,
                                  possiblePort + i,
                                  inetAddress->to_v6().scope_id()));
            }
        }
    }
    // TODO: Add ip v6 addresses using interfaces as done in Java client.

    return addresses;
}

AddressHolder::AddressHolder(const std::string& address,
                             const std::string& scope_id,
                             int port)
  : address_(address)
  , scope_id_(scope_id)
  , port_(port)
{}

std::ostream&
operator<<(std::ostream& os, const AddressHolder& holder)
{
    os << "AddressHolder [" << holder.address_ + "]:" << holder.port_;
    return os;
}

const std::string&
AddressHolder::get_address() const
{
    return address_;
}

const std::string&
AddressHolder::get_scope_id() const
{
    return scope_id_;
}

int
AddressHolder::get_port() const
{
    return port_;
}
} // namespace util
} // namespace hazelcast

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

#define FORCE_INLINE __forceinline

#define ROTL32(x, y) _rotl(x, y)
#define ROTL64(x, y) _rotl64(x, y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else // defined(_MSC_VER)

#define FORCE_INLINE inline __attribute__((always_inline))

FORCE_INLINE uint32_t
rotl32(uint32_t x, int8_t r)
{
    return (x << r) | (x >> (32 - r));
}

FORCE_INLINE uint64_t
rotl64(uint64_t x, int8_t r)
{
    return (x << r) | (x >> (64 - r));
}

#define ROTL32(x, y) rotl32(x, y)
#define ROTL64(x, y) rotl64(x, y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

namespace hazelcast {
namespace util {
FORCE_INLINE uint32_t
getblock32(const uint32_t* p, int i)
{
    return *(p + i);
}

FORCE_INLINE uint64_t
getblock64(const uint64_t* p, int i)
{
    return *(p + i);
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

FORCE_INLINE uint32_t
fmix32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
}

//----------

FORCE_INLINE uint64_t
fmix64(uint64_t k)
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;

    return k;
}

//-----------------------------------------------------------------------------
int
murmur_hash3_x86_32(const void* key, int len)
{
    uint32_t DEFAULT_MURMUR_SEED = 0x01000193;
    uint32_t hash = 0;
    murmur_hash3_x86_32(key, len, DEFAULT_MURMUR_SEED, (void*)&hash);
    return hash;
}

void
murmur_hash3_x86_32(const void* key, int len, uint32_t seed, void* out)
{
    const uint8_t* data = (const uint8_t*)key;
    const int nblocks = len / 4;

    uint32_t h1 = seed;

    const uint32_t c1 = 0xcc9e2d51;
    const uint32_t c2 = 0x1b873593;

    //----------
    // body

    const uint32_t* blocks = (const uint32_t*)(data + nblocks * 4);

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

    const uint8_t* tail = (const uint8_t*)(data + nblocks * 4);

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

    *(uint32_t*)out = h1;
}

//-----------------------------------------------------------------------------
} // namespace util
} // namespace hazelcast
//-----------------------------------------------------------------------------

//  Copyright (c) 2015 ihsan demir. All rights reserved.
//

namespace hazelcast {
namespace util {
const std::shared_ptr<exception_util::runtime_exception_factory>
  exception_util::hazelcastExceptionFactory(
    new class hazelcast_exception_factory());

void
exception_util::rethrow(std::exception_ptr e)
{
    return rethrow(e, hazelcast_exception_factory());
}

void
exception_util::rethrow(
  std::exception_ptr e,
  const std::shared_ptr<exception_util::runtime_exception_factory>&
    runtime_exception_factory)
{
    try {
        std::rethrow_exception(e);
    } catch (client::exception::iexception& ie) {
        if (ie.is_runtime()) {
            std::rethrow_exception(e);
        }

        int32_t errorCode = ie.get_error_code();
        if (errorCode == client::protocol::EXECUTION) {
            try {
                std::rethrow_if_nested(ie);
            } catch (...) {
                rethrow(std::current_exception(), runtime_exception_factory);
            }
        }

        runtime_exception_factory->rethrow(e, "");
    }
}

const std::shared_ptr<exception_util::runtime_exception_factory>&
exception_util::hazelcast_exception_factory()
{
    return hazelcastExceptionFactory;
}

exception_util::runtime_exception_factory::~runtime_exception_factory() =
  default;

void
exception_util::hazelcast_exception_factory::rethrow(
  std::exception_ptr throwable,
  const std::string& message)
{
    try {
        std::rethrow_exception(throwable);
    } catch (...) {
        std::throw_with_nested(boost::enable_current_exception(
          client::exception::hazelcast_("hazelcast_Factory::create", message)));
    }
}
} // namespace util
} // namespace hazelcast

namespace hazelcast {
namespace util {

ByteBuffer::ByteBuffer(char* buffer, size_t capacity)
  : pos_(0)
  , lim_(capacity)
  , capacity_(capacity)
  , buffer_(buffer)
{}

ByteBuffer&
ByteBuffer::flip()
{
    lim_ = pos_;
    pos_ = 0;
    return *this;
}

ByteBuffer&
ByteBuffer::compact()
{
    memcpy(buffer_, ix(), (size_t)remaining());
    pos_ = remaining();
    lim_ = capacity_;
    return *this;
}

ByteBuffer&
ByteBuffer::clear()
{
    pos_ = 0;
    lim_ = capacity_;
    return *this;
}

size_t
ByteBuffer::remaining() const
{
    return lim_ - pos_;
}

bool
ByteBuffer::has_remaining() const
{
    return pos_ < lim_;
}

size_t
ByteBuffer::position() const
{
    return pos_;
}

int
ByteBuffer::read_int()
{
    char a = read_byte();
    char b = read_byte();
    char c = read_byte();
    char d = read_byte();
    return (0xff000000 & (a << 24)) | (0x00ff0000 & (b << 16)) |
           (0x0000ff00 & (c << 8)) | (0x000000ff & d);
}

void
ByteBuffer::write_int(int v)
{
    write_byte(char(v >> 24));
    write_byte(char(v >> 16));
    write_byte(char(v >> 8));
    write_byte(char(v));
}

short
ByteBuffer::read_short()
{
    byte a = read_byte();
    byte b = read_byte();
    return (short)((0xff00 & (a << 8)) | (0x00ff & b));
}

void
ByteBuffer::write_short(short v)
{
    write_byte(char(v >> 8));
    write_byte(char(v));
}

byte
ByteBuffer::read_byte()
{
    byte b = (byte)buffer_[pos_];
    safe_increment_position(1);
    return b;
}

void
ByteBuffer::write_byte(char c)
{
    buffer_[pos_] = c;
    safe_increment_position(1);
}

void*
ByteBuffer::ix() const
{
    return (void*)(buffer_ + pos_);
}

void
ByteBuffer::safe_increment_position(size_t t)
{
    assert(pos_ + t <= capacity_);
    pos_ += t;
}

hz_thread_pool::hz_thread_pool()
  : pool_(new boost::asio::thread_pool())
{}

hz_thread_pool::hz_thread_pool(size_t num_threads)
  : pool_(new boost::asio::thread_pool(num_threads))
{}

void
hz_thread_pool::close()
{
    bool expected = false;
    if (!closed_.compare_exchange_strong(expected, true)) {
        return;
    }
    pool_->join();
}

bool
hz_thread_pool::closed()
{
    return closed_;
}

void
hz_thread_pool::submit(boost::executors::work&& closure)
{
    boost::asio::post(*pool_, closure);
}

bool
hz_thread_pool::try_executing_one()
{
    return true;
}

boost::asio::thread_pool::executor_type
hz_thread_pool::get_executor() const
{
    return pool_->get_executor();
}
} // namespace util
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
