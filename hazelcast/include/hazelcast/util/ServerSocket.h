//
// Created by sancar koyunlu on 30/12/13.
//


#ifndef HAZELCAST_ServerSocket
#define HAZELCAST_ServerSocket

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>
#endif

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/util/AtomicBoolean.h"

namespace hazelcast {
    namespace client{
        class Socket;
    }
    namespace util {
        class ServerSocket {
        public:
            ServerSocket(int port = 0);

            ~ServerSocket();

            client::Socket *accept();

            int getPort() const;

            void close();

			bool isIpv4() const;

        private:
            int socketId;
			bool ipv4;
            util::AtomicBoolean isOpen;

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			WSADATA wsa_data;
			#endif

        };
    }
}


#endif //HAZELCAST_ServerSocket

