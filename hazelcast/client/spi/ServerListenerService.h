//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.



#ifndef HAZELCAST_SERVER_LISTENER_SERVICE
#define HAZELCAST_SERVER_LISTENER_SERVICE

#include "../serialization/Data.h"
#include "../../util/ConcurrentMap.h"
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
                    util::ConcurrentMap<long, ListenerSupportBase> *m = new util::ConcurrentMap<long, ListenerSupportBase>;
                    ListenerSupportBase *listenerSupport = new ListenerSupport<Request, EventHandler, Event >(invocationService, registrationRequest, eventHandler, key);
                    long registrationId = reinterpret_cast<long>(listenerSupport);
                    m->put(registrationId, listenerSupport);

                    util::ConcurrentMap<long, ListenerSupportBase> *oldMap = allListeners.putIfAbsent(instanceName, m);
                    if (oldMap != NULL) {
                        delete m;
                        oldMap->put(registrationId, listenerSupport);
                    }
                    return registrationId;
                };

                template <typename Request, typename EventHandler, typename Event>
                long listen(const std::string& instanceName, const Request& registrationRequest, const EventHandler& eventHandler) {
                    util::ConcurrentMap<long, ListenerSupportBase> *m = new util::ConcurrentMap<long, ListenerSupportBase>;
                    ListenerSupportBase *listenerSupport = new ListenerSupport<Request, EventHandler, Event >(invocationService, registrationRequest, eventHandler);
                    long registrationId = reinterpret_cast<long>(listenerSupport);
                    m->put(registrationId, listenerSupport);

                    util::ConcurrentMap<long, ListenerSupportBase> *oldMap = allListeners.putIfAbsent(instanceName, m);
                    if (oldMap != NULL) {
                        delete m;
                        oldMap->put(registrationId, listenerSupport);
                    }
                    return registrationId;
                };

                bool stopListening(const std::string& instanceName, long registrationId);

            private:
                util::ConcurrentMap<std::string, util::ConcurrentMap<long, ListenerSupportBase> > allListeners;
                InvocationService& invocationService;
            };
        }
    }
}

#endif //__ServerListenerService_H_
