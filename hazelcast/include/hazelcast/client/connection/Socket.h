#ifndef HAZELCAST_SOCKET
#define HAZELCAST_SOCKET


#include "../Address.h"

#ifdef WIN32
#include <winsock.h>
#else

#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/errno.h>

#endif


namespace hazelcast {

    typedef unsigned char byte;

    namespace client {
        namespace connection {

            class Socket {
            public:

                Socket(const Address &address);

                ~Socket();

                void connect();

                void send(const void *buffer, int len) const;

                void receive(void *buffer, int len) const;

                std::string getHost() const;

                int getPort() const;

                int getSocketId() const;

                void close();

            private:

                Socket(const Socket &rhs);

                void getInfo(const Address &address);

                Address address;
                struct addrinfo *server_info;
                int socketId;
                int size;


#ifdef WIN32
			        WSADATA wsa_data;
		        #endif

            };

        }
    }
}

#endif /* HAZELCAST_SOCKET */