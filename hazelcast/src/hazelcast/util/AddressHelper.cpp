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

#include "hazelcast/util/AddressUtil.h"
#include "hazelcast/client/exception/IllegalStateException.h"
#include "hazelcast/util/ILogger.h"
#include "hazelcast/util/AddressHelper.h"

#include <asio/ip/tcp.hpp>

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
        AddressHelper::getPossibleSocketAddresses(int port, const std::string &scopedAddress, int portTryCount, ILogger &logger) {
            std::unique_ptr<asio::ip::address> inetAddress;
            try {
                inetAddress.reset(new asio::ip::address(AddressUtil::getByName(scopedAddress)));
            } catch (client::exception::UnknownHostException &ignored) {
                logger.finest() << "Address " << scopedAddress << " ip number is not available"
                                                          << ignored.what();
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
