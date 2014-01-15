#ifndef HAZELCAST_SOCKET
#define HAZELCAST_SOCKET


#include "hazelcast/client/Address.h"
#include <string>
#include <boost/atomic.hpp>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
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
#include <sys/select.h>


#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    typedef int socklen_t;
#endif

#if !defined(MSG_NOSIGNAL)
#  define MSG_NOSIGNAL 0
#endif

namespace hazelcast {
    namespace client {
        class HAZELCAST_API Socket {
        public:

            Socket(struct addrinfo *serverInfo, int socketId);

            Socket(const client::Address &address);

            ~Socket();

            int connect();

            int send(const void *buffer, int len) const;

            int receive(void *buffer, int len, int flag = 0) const;

            int getSocketId() const;

            client::Address getAddress() const;

            void close();

        private:

            Socket(const Socket &rhs);

            struct addrinfo *serverInfo;
            int socketId;
            boost::atomic<bool> isOpen;

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			WSADATA wsa_data;
			#endif

        };

        struct HAZELCAST_API socketPtrComp {
            bool operator ()(Socket const *const &lhs, Socket const *const &rhs) const {
                return lhs->getSocketId() > rhs->getSocketId();
            }
        };

    }
}

#endif /* HAZELCAST_SOCKET */