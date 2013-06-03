#include <string>
#include "Address.h"

namespace hazelcast {
    namespace client {

        Address::Address(std::string url, int port)
        : host(url), port(port), type(IPv4) {

        };

        Address::Address(const Address& rhs)
        : host(rhs.host), port(rhs.port), type(rhs.type) {

        };


        Address & Address::operator = (Address const & address) {
            host = address.host;
            port = address.port;
            type = address.type;
            return (*this);
        };

        bool Address::operator ==(const hazelcast::client::Address& rhs) const {
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