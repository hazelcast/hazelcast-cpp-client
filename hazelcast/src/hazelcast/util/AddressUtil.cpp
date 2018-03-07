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

#include "hazelcast/util/AddressUtil.h"

namespace hazelcast {
    namespace util {

        AddressHolder AddressUtil::getAddressHolder(const std::string &address, int defaultPort) {
            std::string::size_type indexBracketStart = address.find('[');
            std::string::size_type indexBracketEnd = address.find(']', indexBracketStart);
            std::string::size_type indexColon = address.find(':');
            std::string::size_type lastIndexColon = address.rfind(':');
            std::string host;
            int port = defaultPort;
            std::string scopeId;
            if (indexColon != std::string::npos && lastIndexColon > indexColon) {
                // IPv6
                if (indexBracketStart == 0 && indexBracketEnd > indexBracketStart) {
                    host = address.substr(indexBracketStart + 1, indexBracketEnd - (indexBracketStart + 1));
                    if (lastIndexColon == indexBracketEnd + 1) {
                        port = atoi(address.substr(lastIndexColon + 1).c_str());
                    }
                } else {
                    host = address;
                }
                std::string::size_type indexPercent = host.find('%');
                if (indexPercent != std::string::npos) {
                    scopeId = host.substr(indexPercent + 1);
                    host = host.substr(0, indexPercent);
                }
            } else if (indexColon != 0 && indexColon == lastIndexColon) {
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
    }
}
