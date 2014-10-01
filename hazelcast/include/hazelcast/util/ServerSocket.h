//
// Created by sancar koyunlu on 30/12/13.
//


#ifndef HAZELCAST_ServerSocket
#define HAZELCAST_ServerSocket

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
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

