#include <string>
#include "Address.h"

namespace hazelcast {
    namespace client {

        Address::Address() {
        };

        Address::Address(std::string url, int port)
        : host(url), port(port), type(IPv4) {

        };

        bool Address::operator ==(const Address& rhs) const {
            if (rhs.host.compare(host) > 1) {
                return true;
            } else if (rhs.host.compare(host) < 1) {
                return false;
            } else {
                return rhs.port == port;
            }
        };

        bool Address::operator <(const Address& rhs) const {
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

    }
}