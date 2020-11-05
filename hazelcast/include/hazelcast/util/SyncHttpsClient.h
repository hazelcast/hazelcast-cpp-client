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
#pragma once


#include <string>
#include <boost/asio.hpp>

#ifdef HZ_BUILD_WITH_SSL
#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#endif // HZ_BUILD_WITH_SSL

#include "hazelcast/util/HazelcastDll.h"

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#pragma warning(disable: 4003) //for  not enough actual parameters for macro 'min' in asio wait_traits
#endif

namespace hazelcast {
    namespace util {
        class HAZELCAST_API SyncHttpsClient {
        public:
            SyncHttpsClient(const std::string &serverIp, const std::string &uriPath);

            std::istream &open_connection();
        private:
            std::string server_;
            std::string uriPath_;

            boost::asio::io_service ioService_;

            #ifdef HZ_BUILD_WITH_SSL
            boost::asio::ssl::context sslContext_;
            std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket> > socket_;
            #endif // HZ_BUILD_WITH_SSL

            boost::asio::streambuf response_;
            std::istream responseStream_;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif



