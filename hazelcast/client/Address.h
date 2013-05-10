#ifndef HAZELCAST_ADDRESS
#define HAZELCAST_ADDRESS

#include<string>

namespace hazelcast {
    namespace client {

        class Address {
        public:

            Address(std::string url, std::string port);

            Address(const Address&);

            std::string getPort() const;

            std::string getAddress() const;

        private:
            std::string url;
            std::string port;
        };

    }
}

#endif /* HAZELCAST_ADDRESS */