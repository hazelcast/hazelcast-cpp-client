//
// Created by sancar koyunlu on 14/01/14.
//


#ifndef HAZELCAST_BaseEventHandler
#define HAZELCAST_BaseEventHandler

#include "hazelcast/util/HazelcastDll.h"
#include <string>

namespace hazelcast {
    namespace client {
        namespace serialization {
            class Data;
        }
        namespace impl {
            class HAZELCAST_API BaseEventHandler {
            public:
                virtual ~BaseEventHandler(){

                };
                
                virtual void handle(const client::serialization::Data &data) = 0;

                std::string registrationId;
            };
        }
    }
}
#endif //HAZELCAST_BaseEventHandler
