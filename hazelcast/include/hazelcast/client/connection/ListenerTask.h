//
// Created by sancar koyunlu on 24/12/13.
//




#ifndef HAZELCAST_ListenerTask
#define HAZELCAST_ListenerTask

namespace hazelcast {
    namespace client {
        namespace connection {
            class IOListener;

            class ListenerTask {
            public:
                void init(IOListener *nioListener) {
                    this->nioListener = nioListener;
                };

                virtual void process() = 0;

                virtual ~ListenerTask() {

                };
            protected:
                IOListener *nioListener;
            };
        }
    }
}


#endif //HAZELCAST_ListenerTask
