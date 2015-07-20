#ifndef HAZELCAST_SOCKET
#define HAZELCAST_SOCKET


#include "hazelcast/client/Address.h"
#include "hazelcast/util/AtomicBoolean.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export	
#endif 

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
#include <fcntl.h>

#endif


#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    typedef int socklen_t;
#endif

#if !defined(MSG_NOSIGNAL)
#  define MSG_NOSIGNAL 0
#endif

namespace hazelcast {
    namespace client {
        /**
         * c Sockets wrapper class.
         */
        class HAZELCAST_API Socket {
        public:
            /**
             * Constructor
             */
            Socket(int socketId);
            /**
             * Constructor
             */
            Socket(const client::Address &address);
            /**
             * Destructor
             */
            ~Socket();
            /**
             * connects to given address in constructor.
             * @param timeoutInMillis if not connected within timeout, it will return errorCode
             * @return zero if error. -1 otherwise.
             */
            int connect(int timeoutInMillis);

            /**
             * @param buffer
             * @param len length of the buffer
             * @return number of bytes send
             * @throw IOException in failure.
             */
            int send(const void *buffer, int len) const;

            /**
             * @param buffer
             * @param len  length of the buffer to be received.
             * @param flag bsd sockets options flag.
             * @return number of bytes received.
             * @throw IOException in failure.
             */
            int receive(void *buffer, int len, int flag = 0) const;

            /**
             * return socketId
             */
            int getSocketId() const;

            /**
             * @param address remote endpoint address.
             */
            void setRemoteEndpoint(client::Address &address);

            /**
             * @return remoteEndpoint
             */
            const client::Address &getRemoteEndpoint() const;
            /**
             * closes the socket. Automatically called in destructor.
             * Second call to this function is no op.
             */
            void close();

            client::Address getAddress() const;

            void setBlocking(bool blocking);

        private:

            Socket(const Socket &rhs);

            Socket& operator=(const Socket &rhs);

            client::Address remoteEndpoint;

            struct addrinfo *serverInfo;
            int socketId;
			util::AtomicBoolean isOpen;

			#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			WSADATA wsa_data;
			#endif

        };

        /**
         * Socket Ptr compare method. Compares based on socket id.
         */
        struct HAZELCAST_API socketPtrComp {
            /**
             * Functor.
             */
            bool operator ()(Socket const *const &lhs, Socket const *const &rhs) const {
                return lhs->getSocketId() > rhs->getSocketId();
            }
        };

    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif 

#endif /* HAZELCAST_SOCKET */
