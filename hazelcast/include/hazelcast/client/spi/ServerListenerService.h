//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERVER_LISTENER_SERVICE
#define HAZELCAST_SERVER_LISTENER_SERVICE

#include "hazelcast/util/SynchronizedMap.h"
#include "hazelcast/util/HazelcastDll.h"

namespace hazelcast {
    namespace client {
        namespace impl {
            class PortableRequest;

            class EventHandlerWrapper;
        }

        namespace serialization {
            class Data;
        }
        namespace spi {
            class ClientContext;

            class HAZELCAST_API ServerListenerService {
            public:
                ServerListenerService(spi::ClientContext &clientContext);

                std::string listen(const impl::PortableRequest *registrationRequest, const serialization::Data *partitionKey, impl::EventHandlerWrapper *handler);

                std::string listen(const impl::PortableRequest *registrationRequest, impl::EventHandlerWrapper *handler);

                bool stopListening(const impl::PortableRequest *request, const std::string &registrationId);

                void reRegisterListener(const std::string &uuid,boost::shared_ptr<std::string> alias, int callId);
            private:
                util::SynchronizedMap<std::string, int > registrationIdMap;
                util::SynchronizedMap<std::string, const std::string > registrationAliasMap;
                spi::ClientContext& clientContext;

                void registerListener(boost::shared_ptr<std::string> uuid, int callId);

                bool deRegisterListener(const std::string &uuid);
            };
        }
    }
}

#endif //__ServerListenerService_H_
