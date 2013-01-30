
#include <string>
#include "Address.h"

namespace hazelcast{ 
namespace client{

Address::Address():address(""),port(""){
    
};

Address::Address(std::string address, std::string port):address(address),port(port){
    
};

Address::Address(const Address& rhs){
    address = rhs.address;
    port = rhs.port;
};

Address::~Address(){
    
}

void Address::setPort(std::string port) {
    this->port = port;
}

std::string Address::getPort() const {
    return port;
}

void Address::setAddress(std::string address) {
    this->address = address;
}

std::string Address::getAddress() const {
    return address;
};
    
}}