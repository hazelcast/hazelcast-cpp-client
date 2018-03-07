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
#include <asio/asio/include/asio/io_service.hpp>
#include <asio/asio/include/asio/ip/tcp.hpp>
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/AddressHelper.h"

namespace hazelcast {
    namespace util {
        const int AddressHelper::MAX_PORT_TRIES = 3;
        const int AddressHelper::INITIAL_FIRST_PORT = 5701;

        std::vector<client::Address> AddressHelper::getSocketAddresses(const std::string &address) {
            const AddressHolder addressHolder = AddressUtil::getAddressHolder(address, -1);
            const std::string scopedAddress = !addressHolder.getScopeId().empty()
                                         ? addressHolder.getAddress() + '%' + addressHolder.getScopeId()
                                         : addressHolder.getAddress();

            int port = addressHolder.getPort();
            int maxPortTryCount = 1;
            if (port == -1) {
                maxPortTryCount = MAX_PORT_TRIES;
            }
            return getPossibleSocketAddresses(port, scopedAddress, maxPortTryCount);
        }

        std::vector<client::Address>
        AddressHelper::getPossibleSocketAddresses(int port, const std::string &scopedAddress, int portTryCount) {
            std::string inetAddress;
            bool isIpV4;
            try {
                asio::io_service ioService;
                asio::ip::tcp::resolver res(ioService);
                asio::ip::tcp::resolver::query query(scopedAddress, "");
                asio::ip::basic_resolver<asio::ip::tcp>::iterator iterator = res.resolve(query);
                isIpV4 = iterator->endpoint().address().is_v4();
            } catch (asio::system_error &ignored) {
                util::ILogger::getLogger().getLogger().finest() << "Address not available" << ignored.what();
            }

            int possiblePort = port;
            if (possiblePort == -1) {
                possiblePort = INITIAL_FIRST_PORT;
            }
            std::vector<client::Address> addresses;

            if (inetAddress.empty()) {
                for (int i = 0; i < portTryCount; i++) {
                    addresses.push_back(client::Address(scopedAddress, possiblePort + i));
                }
            } else if (isIpV4) {
                for (int i = 0; i < portTryCount; i++) {
                    //TODO addresses.push_back(client::Address(scopedAddress, inetAddress, possiblePort + i));
                    addresses.push_back(client::Address(inetAddress, possiblePort + i));
                }
            }/* TODO: else if (isIpV6) {
                final Collection<Inet6Address> possibleInetAddresses = getPossibleInetAddressesFor((Inet6Address) inetAddress);
                for (Inet6Address inet6Address : possibleInetAddresses) {
                    for (int i = 0; i < portTryCount; i++) {
                        addresses.add(new Address(scopedAddress, inet6Address, possiblePort + i));
                    }
                }
            }*/

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
