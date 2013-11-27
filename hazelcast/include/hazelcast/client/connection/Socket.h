#ifndef HAZELCAST_SOCKET
#define HAZELCAST_SOCKET


#include "hazelcast/client/Address.h"
#include <string>
#include <boost/atomic.hpp>

#ifdef WIN32
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
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


#ifdef WIN32
    typedef int socklen_t;
#endif

#if !defined(MSG_NOSIGNAL)
#  define MSG_NOSIGNAL 0
#endif

namespace hazelcast {
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

                Address address;
                struct addrinfo *serverInfo;
                int size;
                int socketId;
                boost::atomic<bool> isOpen;

#ifdef WIN32
			        WSADATA wsa_data;
				#endif

            };

        }
    }
}

#endif /* HAZELCAST_SOCKET */