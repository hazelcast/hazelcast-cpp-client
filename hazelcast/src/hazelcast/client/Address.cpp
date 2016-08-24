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
#include "hazelcast/client/cluster/impl/ClusterDataSerializerHook.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {
        const int Address::ID = cluster::impl::ADDRESS;

        const byte Address::IPV4 = 4;
        const byte Address::IPV6 = 6;

        Address::Address():host("localhost"), type(IPV4) {
        }

        Address::Address(const std::string &url, int port)
        : host(url), port(port), type(IPV4) {
        }

        bool Address::operator ==(const Address &rhs) const {
            return rhs.port == port && rhs.type == type && 0 == rhs.host.compare(host);
        }

        bool Address::operator !=(const Address &rhs) const {
            return !(*this == rhs);
        }

        int Address::getPort() const {
            return port;
        }

        const std::string& Address::getHost() const {
            return host;
        }

        int Address::getFactoryId() const {
            return cluster::impl::F_ID;
        }

        int Address::getClassId() const {
            return ID;
        }

        void Address::writeData(serialization::ObjectDataOutput &out) const {
            out.writeInt(port);
            out.writeByte(type);
            int len = (int)host.size();
            out.writeInt(len);
            out.writeBytes((const byte *)host.c_str(), len);
        }

        void Address::readData(serialization::ObjectDataInput &in) {
            port = in.readInt();
            type = in.readByte();
            int len = in.readInt();
            if (len > 0) {
                std::vector<byte> bytes;
                in.readFully(bytes);
                host.clear();
                host.append(bytes.begin(), bytes.end());
            }
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
