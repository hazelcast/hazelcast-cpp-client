#ifndef HAZELCAST_ADDRESS
#define HAZELCAST_ADDRESS

#include "../util/Util.h"
#include "DataSerializable.h"
#include <string>
#include <sstream>
#include <iterator>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class BufferedDataInput;

            class BufferedDataOutput;
        }

        class Address : public DataSerializable {
        public:
            //TODO type IPV4 IPV6
            Address();

            Address(std::string url, int port);

            bool operator == (const Address&) const;

            bool operator <(const Address&) const;

            int getPort() const;

            std::string getHost() const;

            int getFactoryId() const;

            int getClassId() const;

            void writeData(serialization::BufferedDataOutput& writer);

            void readData(serialization::BufferedDataInput& reader);

        private:
            static const byte IPv4 = 4;
            static const byte IPv6 = 6;

            std::string host;
            int port;
            byte type;
        };

        struct addressComparator {
            bool operator ()(const Address& lhs, const Address& rhs) const {
                int i = lhs.getHost().compare(rhs.getHost());
                if (i == 0) {
                    return lhs.getPort() > rhs.getPort();
                }
                return i > 0;

            }
        };


    }
};


inline std::ostream& operator <<(std::ostream &strm, const hazelcast::client::Address &a) {
    return strm << std::string("Address[") << a.getHost() << std::string(":") << hazelcast::util::to_string(a.getPort()) << std::string("]");
};

#endif /* HAZELCAST_ADDRESS */