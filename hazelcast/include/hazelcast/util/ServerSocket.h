//
// Created by sancar koyunlu on 30/12/13.
//


#ifndef HAZELCAST_ServerSocket
#define HAZELCAST_ServerSocket

#include "hazelcast/util/HazelcastDll.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace util {

        class HAZELCAST_API ServerSocket {
        public:
            ServerSocket(int port = 0);

            ~ServerSocket();

            client::Socket *accept();

            int getPort() const;

            void close();
        private:
            int socketId;
            boost::atomic<bool> isOpen;

            #if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
			WSADATA wsa_data;
			#endif

        };
    }
}


#endif //HAZELCAST_ServerSocket
