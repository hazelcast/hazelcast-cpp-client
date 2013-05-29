#ifndef HAZELCAST_ADDRESS
#define HAZELCAST_ADDRESS

#include "protocol/ProtocolConstants.h"
#include "serialization/ConstantSerializers.h"
#include "../util/Util.h"
#include <string>
#include <sstream>
#include <iterator>

namespace hazelcast {
    namespace client {

        class Address {
            template<typename HzWriter>
            friend void ::hazelcast::client::serialization::writePortable(HzWriter& writer, const hazelcast::client::Address& ar);

            template<typename HzReader>
            friend void ::hazelcast::client::serialization::readPortable(HzReader& reader, hazelcast::client::Address& ar);

        public:
            //TODO type IPV4 IPV6
            Address(std::string url, int port);

            Address(const Address&);

            Address& operator = (const Address&);

            bool operator <(const Address&) const;

            int getPort() const;

            std::string getHost() const;

        private:
            static const byte IPv4 = 4;
            static const byte IPv6 = 6;

            std::string host;
            int port;
            byte type;
        };

    }
};

inline std::ostream& operator <<(std::ostream &strm, const hazelcast::client::Address &a) {
    return strm << std::string("Address[") << a.getHost() << std::string(":") << hazelcast::util::to_string(a.getPort()) << std::string("]");
};

namespace hazelcast {
    namespace client {
        namespace serialization {

            inline int getTypeId(const hazelcast::client::Address& x) {
                return SerializationConstants::CONSTANT_TYPE_DATA;
            };

            inline int getFactoryId(const hazelcast::client::Address& ar) {
                return hazelcast::client::protocol::ProtocolConstants::DATA_FACTORY_ID;
            };

            inline int getClassId(const hazelcast::client::Address& ar) {
                return hazelcast::client::protocol::ProtocolConstants::ADDRESS_ID;
            };


            template<typename HzWriter>
            inline void writePortable(HzWriter& writer, const hazelcast::client::Address& address) {
//                writer << address.port;
//                writer << address.type;
//                int size = address.host.size();
//                writer << size;
//                if (size != 0) {
//                    std::vector<char> temp(size);
//                    char const *str = address.host.c_str();
//                    temp.insert(temp.begin(), str, str + size);
//                    writer << str;
//                }
            };

            template<typename HzReader>
            inline void readPortable(HzReader& reader, hazelcast::client::Address& address) {
//                reader >> address.port;
//                reader >> address.type;
//                int size;
//                reader >> size;
//                if (size != 0) {
//                    std::vector<char> temp(size);
//                    reader >> temp;
//                    std::ostringstream oss;
//                    std::copy(temp.begin(), temp.end(), std::ostream_iterator<int>(oss));
//                    address.host = oss.str();
//                }
            };

        }
    }
}

#endif /* HAZELCAST_ADDRESS */