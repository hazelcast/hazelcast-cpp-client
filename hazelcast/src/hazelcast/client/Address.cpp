/*
 * Copyright (c) 2008-2015, Hazelcast, Inc. All Rights Reserved.
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
#include "hazelcast/client/Address.h"

namespace hazelcast {
    namespace client {

        Address::Address():host("localhost") {
        }

        Address::Address(const std::string &url, int port)
        : host(url), port(port) {

        }

        bool Address::operator ==(const Address &rhs) const {
            if (rhs.host.compare(host) != 0) {
                return false;
            } else {
                return rhs.port == port;
            }
        }

        int Address::getPort() const {
            return port;
        }

        const std::string& Address::getHost() const {
            return host;
        }

        bool addressComparator::operator ()(const Address &lhs, const Address &rhs) const {
            int i = lhs.getHost().compare(rhs.getHost());
            if (i == 0) {
                return lhs.getPort() > rhs.getPort();
            }
            return i > 0;

        }

        std::ostream &operator <<(std::ostream &stream, const Address &address) {
            return stream << "Address[" << address.getHost() << ":" << address.getPort() << "]";
        }

    }
}
