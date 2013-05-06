#ifndef HAZELCAST_SOCKET2
#define HAZELCAST_SOCKET2


#include "../Address.h"
#include <netdb.h>
#include <string>

namespace hazelcast {

    typedef unsigned char byte;

    namespace client {
        namespace protocol2 {

            class Socket {
            public:

                Socket(Address& address);

                ~Socket();

                void sendData(const void *buffer, int len);

                void recvData(void *buffer, int len);

                std::string readLine();

            private:

                Socket(const Socket& rhs);

                void getInfo();

                Address address;
                struct addrinfo *server_info;
                int socketId;


            };

        }
    }
}

#endif /* HAZELCAST_SOCKET2 */