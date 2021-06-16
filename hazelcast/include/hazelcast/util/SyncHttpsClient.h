/*
 * Copyright (c) 2008-2021, Hazelcast, Inc. All Rights Reserved.
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
#pragma once

#include <string>
#include <boost/asio.hpp>

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>

#include "hazelcast/util/export.h"

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable : 4251) // for dll export
#pragma warning(                                                                                   \
  disable : 4003) // for  not enough actual parameters for macro 'min' in asio wait_traits
#endif

namespace hazelcast {
namespace util {
class HAZELCAST_API SyncHttpsClient
{
public:
    SyncHttpsClient(const std::string& server_ip,
                    const std::string& uri_path,
                    std::chrono::steady_clock::duration timeout);

    std::istream& connect_and_get_response();

private:
    std::string server_;
    std::string uri_path_;
    std::chrono::steady_clock::duration timeout_;

    boost::asio::io_service io_service_;
    boost::asio::ip::tcp::resolver resolver_;

    boost::asio::ssl::context ssl_context_;
    std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> socket_;

    boost::asio::streambuf response_;
    std::istream response_stream_;

    void run(boost::system::error_code& error, std::chrono::steady_clock::duration timeout);

    void close();
};
} // namespace util
} // namespace hazelcast

#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif
