#ifndef HAZELCAST_ADDRESS
#define HAZELCAST_ADDRESS

#include "hazelcast/util/Util.h"
#include "hazelcast/client/serialization/IdentifiedDataSerializable.h"
#include <string>
#include <sstream>
#include <iterator>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 


namespace hazelcast {
    namespace client {

        /**
         * IP Address
         */
        class HAZELCAST_API Address : public serialization::IdentifiedDataSerializable {
        public:
            /**
             * Constructor
             */
            Address();
            /**
             * Copy Constructor
             */
            Address(const Address& );
            /**
             * Constructor
             */
            Address(const std::string &url, int port);

            /**
             * comparison operator
             * @param address to be compared.
             */
            bool operator == (const Address &address) const;

            /**
             * @return port number.
             */
            int getPort() const;

            /**
             * @return host address as string
             */
            const std::string& getHost() const;

            /**
             * @see IdentifiedDataSerializable
             */
            int getFactoryId() const;

            /**
             * @see IdentifiedDataSerializable
             */
            int getClassId() const;

            /**
             * @see IdentifiedDataSerializable
             */
            void writeData(serialization::ObjectDataOutput &writer) const;

            /**
             * @see IdentifiedDataSerializable
             */
            void readData(serialization::ObjectDataInput &reader);

        private:
            static const byte IPv4 = 4;
            static const byte IPv6 = 6;

            std::string host;
            int port;
            byte type;
        };

        /**
         * Address comparator functor
         */
        struct HAZELCAST_API addressComparator {
            /**
             * Address comparator functor
             * @param lhs
             * @param rhs
             */
            bool operator ()(const Address &lhs, const Address &rhs) const {
                int i = lhs.getHost().compare(rhs.getHost());
                if (i == 0) {
                    return lhs.getPort() > rhs.getPort();
                }
                return i > 0;

            }
        };

        std::ostream& operator<<(std::ostream &stream, const Address &address);

    }
};

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_ADDRESS */
