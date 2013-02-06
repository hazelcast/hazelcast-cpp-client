#include "Socket.h"

#include "../Address.h"

#include <string>
#include <iostream>
#include <memory>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <vector>

namespace hazelcast{
namespace client{
namespace protocol{
   
Socket::Socket(Address& address):address(address){
    getInfo();
    socketId = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
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
   close(socketId);
};

void Socket::sendData(const void* buffer, int len){
     if(send(socketId,buffer,len,0) == -1)
         throw "Error at sending";
};

std::string Socket::readLine(){
    std::string line;
    bool lineIsRead = false;
    while(!lineIsRead){
        char current;
        recvData(&current,sizeof(char));
        if(current == '\n'){
            lineIsRead = true;
        }else if(current != '\r'){
            line.push_back(current);
        }
        
    }
    return line;
};

void Socket::recvData(void* buffer, int len){
    int size = recv(socketId, (void *)buffer, len, 0);
    if(size  == -1 )
        throw "Error at reading";
    else if(size == 0){
        throw "Connection closed by remote";
    }
};

void Socket::getInfo(){
    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints)); 
    hints.ai_family = AF_UNSPEC;     
    char ipstr[INET6_ADDRSTRLEN];
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_flags = AI_PASSIVE;     
    if (getaddrinfo(address.getAddress().c_str(), address.getPort().c_str(), &hints, &server_info) != 0) {
//        std::string error = "error "; //TODO
//        throw error;
    }
};
    
}}}