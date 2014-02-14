#include "hazelcast/client/Address.h"
#include "hazelcast/client/protocol/ProtocolConstants.h"
#include "hazelcast/client/serialization/ObjectDataOutput.h"
#include "hazelcast/client/serialization/ObjectDataInput.h"

namespace hazelcast {
    namespace client {

        Address::Address()
        : hash(0),host("") {
        };

        Address::Address(const std::string &url, int port)
        : host(url), port(port), type(IPv4), hash(0) {

        };

        bool Address::operator ==(const Address &rhs) const {
            if (rhs.host.compare(host) != 0) {
                return false;
            } else {
                return rhs.port == port;
            }
        };

        bool Address::operator <(const Address &rhs) const {
            int cmpResult = host.compare(rhs.host);
            if (cmpResult > 1) {
                return true;
            } else if (cmpResult < 1) {
                return false;
            } else {
                return port < rhs.port;
            }
        };

        int Address::getPort() const {
            return port;
        };

        std::string Address::getHost() const {
            return host;
        };

        int Address::getFactoryId() const {
            return protocol::ProtocolConstants::DATA_FACTORY_ID;
        };

        int Address::getClassId() const {
            return protocol::ProtocolConstants::ADDRESS_ID;
        };

        void Address::writeData(serialization::ObjectDataOutput &writer) const {
            writer.writeInt(port);
            writer.writeByte(type);
            int size = host.size();
            writer.writeInt(size);
            if (size != 0) {
                std::vector<byte> temp;
                char const *str = host.c_str();
                temp.insert(temp.begin(), str, str + size);
                writer.write(temp);
            }
        };

        void Address::readData(serialization::ObjectDataInput &reader) {
            port = reader.readInt();
            type = reader.readByte();
            int size = reader.readInt();
            if (size != 0) {
                std::vector<byte> temp(size);
                reader.readFully(temp);
                std::ostringstream oss;
                std::copy(temp.begin(), temp.end(), std::ostream_iterator<byte>(oss));
                host = oss.str();
            }
        };


    }
}