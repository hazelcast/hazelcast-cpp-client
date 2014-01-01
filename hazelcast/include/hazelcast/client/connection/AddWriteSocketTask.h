//
// Created by sancar koyunlu on 24/12/13.
//


#ifndef HAZELCAST_AddWriteSocketTask
#define HAZELCAST_AddWriteSocketTask

#include "hazelcast/client/connection/ListenerTask.h"
#include "IOListener.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class AddWriteSocketTask : public ListenerTask {
            public:
                AddWriteSocketTask(const Socket& socket)
                :socket(socket){

                }
                void run() {
                    nioListener->addSocket(socket);
                }


            private:
                const Socket& socket;
            };

        }
    }
}

#endif //HAZELCAST_AddWriteSocketTask
