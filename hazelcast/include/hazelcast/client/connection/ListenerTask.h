//
// Created by sancar koyunlu on 24/12/13.
//




#ifndef HAZELCAST_ListenerTask
#define HAZELCAST_ListenerTask

namespace hazelcast {
    namespace client {
        namespace connection {
            class NIOListener;

            class ListenerTask {
            public:
                void init(NIOListener *nioListener) {
                    this->nioListener = nioListener;
                };

                virtual void process() = 0;

            protected:
                NIOListener *nioListener;
            };
        }
    }
}


#endif //HAZELCAST_ListenerTask
