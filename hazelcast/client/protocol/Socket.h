#ifndef HAZELCAST_SOCKET
#define HAZELCAST_SOCKET


#include "../Address.h"
#include <netdb.h>

namespace hazelcast{

typedef unsigned char byte;    
    
namespace client{
namespace protocol{
   
    
class Socket{
public:
    Socket(Address& address);
    ~Socket();
    void sendData(byte* buffer, int size);
    void recvData(byte* buffer, int size);
private:
    Socket(const Socket& rhs);
    void getInfo();
    Address address;
    struct addrinfo * server_info;
    int socketId;
};   
    
}}}

#endif /* HAZELCAST_SOCKET */