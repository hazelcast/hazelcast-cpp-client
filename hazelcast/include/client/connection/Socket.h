#ifndef HAZELCAST_SOCKET
#define HAZELCAST_SOCKET


#include "../Address.h"
#include <netdb.h>

namespace hazelcast {

    typedef unsigned char byte;

    namespace client {
        namespace connection {

            class Socket {
            public:

                Socket(const Address& address);

                ~Socket();

                void connect();

                void send(const void *buffer, int len);

                void receive(void *buffer, int len);

                std::string getHost() const;

                int getPort() const;

                int getSocketId() const;

                void close();

            private:

                Socket(const Socket& rhs);

                void getInfo(const Address& address);

                Address address;
                struct addrinfo *server_info;
                int socketId;
                int size;


            };

        }
    }
}

#endif /* HAZELCAST_SOCKET */