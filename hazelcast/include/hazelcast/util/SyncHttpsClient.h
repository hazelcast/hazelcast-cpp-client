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
#ifndef HAZELCAST_UTIL_SYNCHTTPSCLIENT_H_
#define HAZELCAST_UTIL_SYNCHTTPSCLIENT_H_


#include <string>
#include <asio.hpp>

#ifdef HZ_BUILD_WITH_SSL
#include <asio/ssl/context.hpp>
#include <asio/ssl/stream.hpp>
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

            std::istream &openConnection();
        private:
            std::string server;
            std::string uriPath;

            asio::io_service ioService;

            #ifdef HZ_BUILD_WITH_SSL
            asio::ssl::context sslContext;
            std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket> > socket;
            #endif // HZ_BUILD_WITH_SSL

            asio::streambuf response;
            std::istream responseStream;
        };
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_UTIL_SYNCHTTPSCLIENT_H_

