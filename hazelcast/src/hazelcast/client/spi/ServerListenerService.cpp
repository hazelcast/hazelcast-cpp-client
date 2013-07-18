//
// Created by sancar koyunlu on 6/24/13.
// Copyright (c) 2013 hazelcast. All rights reserved.


#include "hazelcast/client/spi/ServerListenerService.h"


namespace hazelcast {
    namespace client {


        namespace spi {


            ServerListenerService::ServerListenerService(InvocationService& invocationService)
            :invocationService(invocationService) {

            };

            ServerListenerService::~ServerListenerService() {
                vector<util::ConcurrentMap<long, ListenerSupportBase> *> values = allListeners.values();
                values.clear();
                allListeners.clear();
            };

            bool ServerListenerService::stopListening(const std::string& instanceName, long registrationId) {
                util::ConcurrentMap<long, ListenerSupportBase> *pMap = allListeners.get(instanceName);
                ListenerSupportBase *listenerSupportBase = pMap->remove(registrationId);
                if (listenerSupportBase != NULL) {
                    ListenerSupportBase *listenerSupport = listenerSupportBase;
                    listenerSupport->stop();
                    return true;
                }
                return false;
            };
        }
    }
}
