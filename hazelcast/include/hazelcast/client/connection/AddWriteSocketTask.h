//
// Created by sancar koyunlu on 24/12/13.
//


#ifndef HAZELCAST_AddWriteSocketTask
#define HAZELCAST_AddWriteSocketTask

#include "hazelcast/client/connection/ListenerTask.h"
#include "hazelcast/client/connection/NIOListener.h"
#include "hazelcast/client/Socket.h"

namespace hazelcast {
    namespace client {
        namespace connection {
            class AddWriteSocketTask : public ListenerTask {
            public:
                AddWriteSocketTask(Socket& socket)
                :socket(socket){

                }

                void AddWriteSocketTask::process() {
                    nioListener->addWriteSocket(socket);
                }

            private:
                Socket& socket;
            };

        }
    }
}

#endif //HAZELCAST_AddWriteSocketTask
