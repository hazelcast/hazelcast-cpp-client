#include <string>
#include "Address.h"

namespace hazelcast {
    namespace client {

        Address::Address(std::string url, std::string port) : url(url), port(port) {

        };

        Address::Address(const Address& address): url(address.url), port(address.port) {

        };

        std::string Address::getPort() const {
            return port;
        }

        std::string Address::getAddress() const {
            return url;
        };

    }
}