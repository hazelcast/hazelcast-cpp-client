//
// Created by sancar koyunlu on 14/01/14.
//


#ifndef HAZELCAST_BaseEventHandler
#define HAZELCAST_BaseEventHandler

#include "hazelcast/util/HazelcastDll.h"
#include <string>

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(push)
#pragma warning(disable: 4251) //for dll export
#endif

namespace hazelcast {
    namespace client {
        namespace serialization {
            namespace pimpl{
                class Data;
            }
        }
        namespace impl {
            class HAZELCAST_API BaseEventHandler {
            public:
                virtual ~BaseEventHandler(){

                };
                
                virtual void handle(const client::serialization::pimpl::Data &data) = 0;

                std::string registrationId;
            };
        }
    }
}

#if  defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
#pragma warning(pop)
#endif

#endif //HAZELCAST_BaseEventHandler

