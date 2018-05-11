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

#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/util/IOUtil.h"
#include "hazelcast/util/AddressUtil.h"

#include <asio/io_service.hpp>
#include <asio/ip/basic_resolver.hpp>
#include <asio/ip/tcp.hpp>

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

       asio::ip::address AddressUtil::getByName(const std::string &host) {
            return getByName(host, "");
        }

        asio::ip::address AddressUtil::getByName(const std::string &host, const std::string &service) {
            try {
                asio::io_service ioService;
                asio::ip::tcp::resolver res(ioService);
                asio::ip::tcp::resolver::query query(host, service);
                asio::ip::basic_resolver<asio::ip::tcp>::iterator iterator = res.resolve(query);
                return iterator->endpoint().address();
            } catch (asio::system_error &e) {
                std::ostringstream out;
                out << "Address " << host << " ip number is not available. " << e.what();
                throw client::exception::UnknownHostException("AddressUtil::getByName", out.str());
            }
        }

    }
}
