#ifndef HAZELCAST_ADDRESS
#define HAZELCAST_ADDRESS

#include<string>

namespace hazelcast {
    namespace client {

        class Address {
        public:

            Address();

            Address(std::string address, std::string port);

            void setPort(std::string port);

            std::string getPort() const;

            void setAddress(std::string address);

            std::string getAddress() const;

        private:
            std::string address;
            std::string port;
        };

    }
}

#endif /* HAZELCAST_ADDRESS */