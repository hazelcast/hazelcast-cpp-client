//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERVER_LISTENER_SERVICE
#define HAZELCAST_SERVER_LISTENER_SERVICE

#include "../serialization/Data.h"
#include "ListenerSupport.h"
#include "ClientContext.h"
#include "InvocationService.h"
#include <set>

namespace hazelcast {
    namespace client {
        namespace spi {

            class ServerListenerService {
            public:
                ServerListenerService(InvocationService& invocationService);

                ~ServerListenerService();

                template <typename Request, typename EventHandler, typename Event>
                long listen(const Request& registrationRequest, const serialization::Data& key, const EventHandler& eventHandler) {
                    ListenerSupportBase *listenerSupport = new ListenerSupport<Request, EventHandler, Event >(invocationService, registrationRequest, eventHandler, key);
                    long registrationId = reinterpret_cast<long>(listenerSupport);
                    lock.lock();
                    allListeners[registrationId] = listenerSupport;
                    listenerSupport->listen();
                    lock.unlock();
                    return registrationId;
                };

                template <typename Request, typename EventHandler, typename Event>
                long listen(const Request& registrationRequest, const EventHandler& eventHandler) {
                    ListenerSupport <Request, EventHandler, Event> *support = new ListenerSupport<Request, EventHandler, Event >(invocationService, registrationRequest, eventHandler);
                    ListenerSupportBase *listenerSupport = support;
                    long registrationId = reinterpret_cast<long>(listenerSupport);
                    lock.lock();

                    allListeners[registrationId] = listenerSupport;
                    listenerSupport->listen();
                    lock.unlock();
                    return registrationId;
                };

                bool stopListening(long registrationId);

            private:
                std::map<long, ListenerSupportBase *> allListeners;
                boost::mutex lock;
                InvocationService& invocationService;
            };
        }
    }
}

#endif //__ServerListenerService_H_
