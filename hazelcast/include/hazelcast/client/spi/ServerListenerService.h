//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERVER_LISTENER_SERVICE
#define HAZELCAST_SERVER_LISTENER_SERVICE

#include "../serialization/Data.h"
#include "ConcurrentSmartMap.h"
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
                long listen(const std::string& instanceName, const Request& registrationRequest, const serialization::Data& key, const EventHandler& eventHandler) {
                    ListenerSupportBase *listenerSupport = new ListenerSupport<Request, EventHandler, Event >(invocationService, registrationRequest, eventHandler, key);
                    long registrationId = reinterpret_cast<long>(listenerSupport);

                    util::AtomicPointer<util::ConcurrentSmartMap<long, ListenerSupportBase> > m(new util::ConcurrentSmartMap<long, ListenerSupportBase>);
                    m->put(registrationId, listenerSupport);

                    util::AtomicPointer<util::ConcurrentSmartMap<long, ListenerSupportBase> > oldMap = allListeners.putIfAbsent(instanceName, m);
                    if (oldMap != NULL) {
                        m.reset();
                        oldMap->put(registrationId, listenerSupport);
                    }
                    listenerSupport->listen();
                    return registrationId;
                };

                template <typename Request, typename EventHandler, typename Event>
                long listen(const std::string& instanceName, const Request& registrationRequest, const EventHandler& eventHandler) {
                    ListenerSupportBase *listenerSupport = new ListenerSupport<Request, EventHandler, Event >(invocationService, registrationRequest, eventHandler);
                    long registrationId = reinterpret_cast<long>(listenerSupport);

                    util::AtomicPointer<util::ConcurrentSmartMap<long, ListenerSupportBase> > m(new util::ConcurrentSmartMap<long, ListenerSupportBase>);
                    m->put(registrationId, listenerSupport);

                    util::AtomicPointer<util::ConcurrentSmartMap<long, ListenerSupportBase> > oldMap = allListeners.putIfAbsent(instanceName, m);
                    if (oldMap != NULL) {
                        m.reset();
                        oldMap->put(registrationId, listenerSupport);
                    }
                    listenerSupport->listen();
                    return registrationId;
                };

                bool stopListening(const std::string& instanceName, long registrationId);

            private:
                util::ConcurrentSmartMap<std::string, util::ConcurrentSmartMap<long, ListenerSupportBase> > allListeners;
                InvocationService& invocationService;
            };
        }
    }
}

#endif //__ServerListenerService_H_
