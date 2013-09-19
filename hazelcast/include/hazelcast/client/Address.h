#ifndef HAZELCAST_ADDRESS
#define HAZELCAST_ADDRESS

#include "../util/Util.h"
#include "IdentifiedDataSerializable.h"
#include <string>
#include <sstream>
#include <iterator>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class ObjectDataInput;

            class ObjectDataOutput;
        }

        class Address : public IdentifiedDataSerializable {
        public:
            Address();

            Address(std::string url, int port);

            bool operator == (const Address&) const;

            bool operator <(const Address&) const;

            int getPort() const;

            std::string getHost() const;

            int getFactoryId() const;

            int getClassId() const;

            void writeData(serialization::ObjectDataOutput& writer) const;

            void readData(serialization::ObjectDataInput& reader);

            int hashCode() const;

        private:
            static const byte IPv4 = 4;
            static const byte IPv6 = 6;

            std::string host;
            int port;
            byte type;
            mutable int hash;
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


        inline std::ostream& operator <<(std::ostream &strm, const Address &a) {
            return strm << "Address[" << a.getHost() << ":" << util::to_string(a.getPort()) << "]";
        };
    }
};


#endif /* HAZELCAST_ADDRESS */