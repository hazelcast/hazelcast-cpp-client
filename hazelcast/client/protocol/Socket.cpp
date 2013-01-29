#include "Socket.h"
#include "../Address.h"

#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace hazelcast{
namespace client{
namespace protocol{
   
Socket::Socket(Address& address){
    getInfo();
    socketId = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
    std::cout << "trying to connect  to " << address.getAddress() << ":" << address.getPort() << std::endl;
    if(connect(socketId, server_info->ai_addr, server_info->ai_addrlen) == -1)
        std::cout << "connection error" << std::endl;
};

Socket::Socket(const Socket& rhs){
   //private
   address = rhs.address;  
   socketId = rhs.socketId;
};

Socket::~Socket(){
   freeaddrinfo(server_info);      
};

void Socket::sendData(byte* buffer, int size){
    
};

void Socket::recvData(byte* buffer, int size){
    
};

void Socket::getInfo(){
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints)); 
    hints.ai_family = AF_UNSPEC;     
    char ipstr[INET6_ADDRSTRLEN];
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE;     
    
    if (getaddrinfo(address.getAddress().c_str(), address.getPort().c_str(), &hints, &server_info) != 0) {
        std::string error = "error "; //TODO
        throw error;
    }
};
    
}}}