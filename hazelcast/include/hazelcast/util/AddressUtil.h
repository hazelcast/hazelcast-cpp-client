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

#ifndef HAZELCAST_UTIL_ADDRESSUTIL_H_
#define HAZELCAST_UTIL_ADDRESSUTIL_H_

#include <boost/asio/ip/address.hpp>

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AddressHelper.h"

using namespace boost;

namespace hazelcast {
    namespace util {
        class HAZELCAST_API AddressUtil {
        public:
            static AddressHolder getAddressHolder(const std::string &address);

            static AddressHolder getAddressHolder(const std::string &address, int defaultPort);

            static boost::asio::ip::address getByName(const std::string &host);

        private:
            static boost::asio::ip::address getByName(const std::string &host, const std::string &service);
        };

    }
}


#endif //HAZELCAST_UTIL_ADDRESSUTIL_H_

